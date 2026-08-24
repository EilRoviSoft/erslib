#include "erslib/dbio/connection_pool.hpp"

// std
#include <cstdio>
#include <cstdlib>

// ers
#include <erslib/core/filesystem.hpp>


// ConnectionPool

dbio::impl::ConnectionPool::ConnectionPool(std::string db_connection_string, pool_options_t pool_opts) :
    _db_connection_string(std::move(db_connection_string)),
    _pool_opts(std::move(pool_opts)) {
    _init();
}

ers::Result<dbio::impl::ConnectionPool::Connection> dbio::impl::ConnectionPool::acquire() {
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

    auto conn = _spawn();
    if (!conn)
        return conn.error();

    return Connection(shared_from_this(), *conn);
}

void dbio::impl::ConnectionPool::maintain() {
    std::scoped_lock lock(_mutex);

    std::deque<entry_t> kept;
    const auto now = ers::clock_t::now();

    for (const auto& entry : _idle) {
        if (_is_stale(entry, now) && _all.size() > _pool_opts.min_size)
            _discard(entry.conn);
        else
            kept.emplace_back(entry);
    }

    _idle = std::move(kept);
}


bool dbio::ConnectionPool::_is_stale(const entry_t& entry, ers::timestamp_t now) const noexcept {
    return now - entry.last_used > _pool_opts.idle_timeout;
}


void dbio::impl::ConnectionPool::_init() {
    std::scoped_lock lock(_mutex);
    for (size_t i = 0; i < _pool_opts.min_size; i++) {
        auto conn = _spawn();
        if (!conn)
            break;
        _idle.emplace_back(*conn, ers::clock_t::now());
    }
}

void dbio::impl::ConnectionPool::_release(pqxx::connection* conn) noexcept {
    {
        std::scoped_lock lock(_mutex);
        _idle.emplace_back(conn, ers::clock_t::now());
    }

    _cv.notify_one();
}
void dbio::impl::ConnectionPool::_discard(pqxx::connection* conn) noexcept {
    std::erase_if(_all, [&](const std::unique_ptr<pqxx::connection>& owned) {
        return owned.get() == conn;
    });
}

ers::Result<pqxx::connection*> dbio::impl::ConnectionPool::_spawn() {
    auto conn = std::make_unique<pqxx::connection>(_db_connection_string);

    if (!_trace_file) {
        if (const char* trace_env = std::getenv("DBIO_TRACE_FILE")) {
            fs::path trace_path(trace_env);
            if (trace_path.has_parent_path())
                fs::create_directories(trace_path.parent_path());
            _trace_file.reset(std::fopen(trace_path.string().c_str(), "w"));
        }
    }

    if (_trace_file)
        conn->trace(_trace_file.get());

    auto raw = conn.get();
    _all.emplace_back(std::move(conn));
    return raw;
}


// ConnectionPool::Connection

dbio::impl::ConnectionPool::Connection::Connection(ers::shared_ptr<ConnectionPool> pool, pqxx::connection* conn) noexcept :
    _pool(pool),
    _conn(conn) {
}


dbio::impl::ConnectionPool::Connection::Connection(Connection&& other) noexcept :
    _pool(std::move(other._pool)),
    _conn(std::exchange(other._conn, nullptr)) {
}
dbio::impl::ConnectionPool::Connection& dbio::impl::ConnectionPool::Connection::operator=(Connection&& other) noexcept {
    _release();
    _pool = std::move(other._pool);
    _conn = std::exchange(other._conn, nullptr);
    return *this;
}


dbio::impl::ConnectionPool::Connection::~Connection() { _release(); }


pqxx::connection& dbio::impl::ConnectionPool::Connection::operator*() noexcept { return *_conn; }
pqxx::connection* dbio::impl::ConnectionPool::Connection::operator->() noexcept { return _conn; }


void dbio::impl::ConnectionPool::Connection::_release() noexcept {
    if (!_pool)
        return;

    auto pool = std::exchange(_pool, nullptr);
    auto conn = std::exchange(_conn, nullptr);

    pool->_release(conn);
}
