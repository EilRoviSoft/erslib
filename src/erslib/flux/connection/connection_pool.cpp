#include "erslib/flux/connection/connection_pool.hpp"

// std
#include <cstdio>
#include <cstdlib>
#include <utility>

// ers
#include <erslib/core/exception.hpp>
#include <erslib/core/filesystem.hpp>


// Implementation --------------------------------------------------------------

flux::impl::ConnectionPool::ConnectionPool(std::string db_connection_string, pool_options_t pool_opts) :
    _db_connection_string(std::move(db_connection_string)),
    _pool_opts(std::move(pool_opts)) {
    _init();
}

ers::Result<flux::impl::ConnectionPool::Connection> flux::impl::ConnectionPool::acquire() {
    std::unique_lock lock(_mutex);


    auto can_supply = [this] {
        return !_idle.empty() || _all.size() < _pool_opts.max_size;
    };

    if (!_cv.wait_for(lock, _pool_opts.acquire_timeout, can_supply))
        return ers::make_error("Timed out acquiring a pooled connection");


    const auto now = ers::clock_t::now();

    while (!_idle.empty()) {
        auto entry = _idle.front();
        _idle.pop_front();

        if (!_is_stale(entry, now) && entry.conn->is_open())
            return Connection(shared_from_this(), entry.conn);

        _discard(entry.conn);
    }


    auto conn = ERS_TRY(_spawn());

    return Connection(shared_from_this(), conn);
}
flux::impl::ConnectionPool::Connection flux::impl::ConnectionPool::acquire_or_throw() {
    return acquire().or_throw();
}

void flux::impl::ConnectionPool::maintain() {
    std::scoped_lock lock(_mutex);

    std::deque<entry_t> kept;
    const auto now = ers::clock_t::now();

    for (const auto& entry : _idle) {
        if (_is_stale(entry, now) && _all.size() > _pool_opts.min_size)
            _discard(entry.conn);
        else
            kept.push_back(entry);
    }

    _idle = std::move(kept);
}


bool flux::impl::ConnectionPool::_is_stale(const entry_t& entry, ers::timestamp_t now) const noexcept {
    return now - entry.last_used > _pool_opts.idle_timeout;
}


void flux::impl::ConnectionPool::_init() {
    std::scoped_lock lock(_mutex);
    for (size_t i = 0; i < _pool_opts.min_size; ++i) {
        auto conn = _spawn();
        if (!conn)
            break;
        _idle.emplace_back(*conn, ers::clock_t::now());
    }
}

void flux::impl::ConnectionPool::_release(pqxx::connection* conn) noexcept {
    {
        std::scoped_lock lock(_mutex);
        _idle.emplace_back(conn, ers::clock_t::now());
    }

    _cv.notify_one();
}
void flux::impl::ConnectionPool::_discard(pqxx::connection* conn) noexcept {
    std::erase_if(_all, [&](const std::unique_ptr<pqxx::connection>& owned) {
        return owned.get() == conn;
    });
}

ers::Result<pqxx::connection*> flux::impl::ConnectionPool::_spawn() {
    auto conn = std::make_unique<pqxx::connection>(_db_connection_string);

    if (!_trace_file) {
        if (const char* trace_env = std::getenv("FLUX_TRACE_FILE")) {
            fs::path trace_path(trace_env);
            if (trace_path.has_parent_path())
                fs::create_directories(trace_path.parent_path());
            _trace_file.reset(std::fopen(trace_path.string().c_str(), "w"));
        }
    }

    if (_trace_file)
        conn->trace(_trace_file.get());

    auto raw = conn.get();
    _all.push_back(std::move(conn));
    return raw;
}


// ConnectionPool::Connection

flux::impl::ConnectionPool::Connection::Connection(std::shared_ptr<ConnectionPool> pool, pqxx::connection* conn) noexcept :
    _pool(pool),
    _conn(conn) {
}


flux::impl::ConnectionPool::Connection::Connection(Connection&& other) noexcept :
    _pool(std::move(other._pool)),
    _conn(std::exchange(other._conn, nullptr)) {
}
flux::impl::ConnectionPool::Connection& flux::impl::ConnectionPool::Connection::operator=(Connection&& other) noexcept {
    _release();
    _pool = std::move(other._pool);
    _conn = std::exchange(other._conn, nullptr);
    return *this;
}


flux::impl::ConnectionPool::Connection::~Connection() { _release(); }


pqxx::connection& flux::impl::ConnectionPool::Connection::operator*() noexcept { return *_conn; }
pqxx::connection* flux::impl::ConnectionPool::Connection::operator->() noexcept { return _conn; }


void flux::impl::ConnectionPool::Connection::_release() noexcept {
    if (!_pool)
        return;

    auto pool = std::exchange(_pool, nullptr);
    auto conn = std::exchange(_conn, nullptr);

    pool->_release(conn);
}
