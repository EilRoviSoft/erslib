#include "erslib/dbio/connection_pool.hpp"


// ConnectionPool

dbio::ConnectionPool::ConnectionPool(std::string db_opts, pool_options_t pool_opts) :
    _db_connection_string(std::move(db_opts)),
    _pool_opts(std::move(pool_opts)) {
    _init();
}

ers::Result<dbio::ConnectionPool::Connection> dbio::ConnectionPool::acquire() {
    std::unique_lock lock(_mutex);

    auto can_supply = [this] {
        return !_idle.empty() || _all.size() < _pool_opts.max_size;
    };

    if (!_cv.wait_for(lock, _pool_opts.acquire_timeout, can_supply))
        return ers::make_error("Timed out acquiring a pooled connection");

    while (!_idle.empty()) {
        auto entry = _idle.front();
        _idle.pop_front();

        bool is_stale = ers::clock_t::now() - entry.last_used > _pool_opts.idle_timeout;
        bool is_closed = !entry.conn->is_open();

        if (!is_stale && !is_closed)
            return Connection(shared_from_this(), entry.conn);

        _discard(entry.conn);
    }

    return Connection(shared_from_this(), _spawn());
}

void dbio::ConnectionPool::maintain() {
    std::scoped_lock lock(_mutex);

    std::deque<entry_t> kept;
    auto now = ers::clock_t::now();

    for (const auto& entry : _idle) {
        bool is_stale = now - entry.last_used > _pool_opts.idle_timeout;
        bool is_above_floor = _all.size() > _pool_opts.min_size;

        if (is_stale && is_above_floor)
            _discard(entry.conn);
        else
            kept.emplace_back(entry);
    }

    _idle = std::move(kept);
}


void dbio::ConnectionPool::_init() {
    std::scoped_lock lock(_mutex);
    for (size_t i = 0; i < _pool_opts.min_size; i++)
        _idle.emplace_back(_spawn(), ers::clock_t::now());
}

void dbio::ConnectionPool::_release(pqxx::connection* conn) noexcept {
    {
        std::scoped_lock lock(_mutex);
        _idle.emplace_back(conn, ers::clock_t::now());
    }

    _cv.notify_one();
}
void dbio::ConnectionPool::_discard(pqxx::connection* conn) noexcept {
    std::erase_if(_all, [&](const std::unique_ptr<pqxx::connection>& owned) {
        return owned.get() == conn;
    });
}

pqxx::connection* dbio::ConnectionPool::_spawn() {
    auto conn = std::make_unique<pqxx::connection>(static_cast<std::string>(_db_connection_string));
    pqxx::connection* raw = conn.get();
    _all.emplace_back(std::move(conn));
    return raw;
}


// ConnectionPool::Connection

dbio::ConnectionPool::Connection::Connection(std::shared_ptr<ConnectionPool> pool, pqxx::connection* conn) noexcept :
    _pool(pool),
    _conn(conn) {
}


dbio::ConnectionPool::Connection::Connection(Connection&& other) noexcept :
    _pool(std::move(other._pool)),
    _conn(std::exchange(other._conn, nullptr)) {
}
dbio::ConnectionPool::Connection& dbio::ConnectionPool::Connection::operator=(Connection&& other) noexcept {
    _release();
    _pool = std::move(other._pool);
    _conn = std::exchange(other._conn, nullptr);
    return *this;
}


dbio::ConnectionPool::Connection::~Connection() { _release(); }


pqxx::connection& dbio::ConnectionPool::Connection::operator*() noexcept { return *_conn; }
pqxx::connection* dbio::ConnectionPool::Connection::operator->() noexcept { return _conn; }


void dbio::ConnectionPool::Connection::_release() noexcept {
    if (!_pool)
        return;

    auto pool = std::exchange(_pool, nullptr);
    auto conn = std::exchange(_conn, nullptr);

    pool->_release(conn);
}
