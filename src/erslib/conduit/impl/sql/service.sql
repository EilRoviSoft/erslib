-- name: lock
SELECT pg_advisory_xact_lock(hashtext($1))

-- name: create_schema
CREATE SCHEMA IF NOT EXISTS {0}

-- name: create_migrations
CREATE TABLE IF NOT EXISTS {0}.migrations (
    schema     TEXT NOT NULL,
    id         TEXT NOT NULL,
    version    INTEGER,
    outcome    TEXT NOT NULL,
    checksum   BIGINT NOT NULL,
    applied_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    CONSTRAINT migrations_pk PRIMARY KEY (schema, id)
)

-- name: create_tables
CREATE TABLE IF NOT EXISTS {0}.tables (
    schema      TEXT NOT NULL,
    name        TEXT NOT NULL,
    format      INTEGER NOT NULL,
    hash        BIGINT NOT NULL,
    declaration TEXT NOT NULL,
    updated_at  TIMESTAMPTZ NOT NULL DEFAULT now(),
    CONSTRAINT tables_pk PRIMARY KEY (schema, name)
)

-- name: tracks
SELECT EXISTS (SELECT 1 FROM {0}.tables WHERE schema = $1)

-- name: applied
SELECT id, checksum FROM {0}.migrations WHERE schema = $1

-- name: recorded_tables
SELECT name, format, hash, declaration FROM {0}.tables WHERE schema = $1

-- name: record_migration
INSERT INTO {0}.migrations (schema, id, version, outcome, checksum)
VALUES ($1, $2, $3, $4, $5)
ON CONFLICT (schema, id) DO UPDATE
SET outcome = EXCLUDED.outcome, checksum = EXCLUDED.checksum, applied_at = now()

-- name: remember_table
INSERT INTO {0}.tables (schema, name, format, hash, declaration)
VALUES ($1, $2, $3, $4, $5)
ON CONFLICT (schema, name) DO UPDATE
SET format = EXCLUDED.format, hash = EXCLUDED.hash, declaration = EXCLUDED.declaration, updated_at = now()

-- name: forget_table
DELETE FROM {0}.tables WHERE schema = $1 AND name = $2

-- name: has_table
SELECT to_regclass($1) IS NOT NULL
