# AdequateApi CPP Web Framework

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![Boost.Asio](https://img.shields.io/badge/Boost-Asio-orange)
![Boost.Beast](https://img.shields.io/badge/Boost-Beast-yellow)
![PostgreSQL](https://img.shields.io/badge/PostgreSQL-14%2B-blue)
![Redis](https://img.shields.io/badge/Redis-7%2B-red)
![Dockerized](https://img.shields.io/badge/Docker-Ready-2496ED)
![Sanitizers](https://img.shields.io/badge/ASan-Clean-success)
![Version](https://img.shields.io/badge/version-0.9.0-informational)
![License](https://img.shields.io/badge/license-MIT-green)

Production-oriented C++20 backend framework built on Boost.Asio
coroutines and Boost.Beast.\
Focused on deterministic async execution, explicit lifetimes, and
architectural invariants.

------------------------------------------------------------------------

# Preface

I build this framework for one reason: invariance.

Modern backend stacks optimize for development speed.\
Few optimize for structural correctness under load.

When IO mixes with CPU work, when cancellation happens mid-flight, when
executors are hidden behind abstractions --- systems start behaving
unpredictably.

This framework exists to remove hidden behavior.

-   IO is explicit.
-   CPU work is explicit.
-   Executors are explicit.
-   Lifetimes are deterministic.
-   No hidden concurrency.
-   No implicit thread creation.

It is not about reinventing frameworks.\
It is about building a predictable one.

------------------------------------------------------------------------

# Quick start

## Prerequisites

- Docker + Docker Compose
- `make` (Windows: Git Bash / MSYS2 / WSL — любой вариант, где `make` доступен)

---

## 1) Clone

```bash
git clone https://github.com/Emilianissimo/adequate-api-cpp.git
cd https://github.com/Emilianissimo/adequate-api-cpp.git
```

## 2) Prepare environment

Creates required folders (MEDIA, TEST_MEDIA) and prepares env files from examples:

```bash
make set-env
```

This will:
- create media/ and test_media/ folders
- copy .env.example → .env (if missing)
- copy .env.test.example → .env.test (if missing)

## 3) Build & run (Docker)

Build all services:
```bash
make build
```

Start stack in background:
```bash
make start
```

## 4) Database migrations

Run migrations:

```bash
make migrate
```

Rollback all:

```bash
make rollback
```

## 5) E2E tests

Build and start E2E environment:

```bash
make e2e-test-build
```

Run E2E tests (runs test_e2e container, then removes stack):

```bash
make e2e-test
```
Note (Windows): some shells may not execute multistep recipes reliably.
If needed, run make e2e-test-build, then run the docker compose ... run ... line manually, then make e2e-test-remove.

------------------------------------------------------------------------

# Features (v0.9.0)

## Core HTTP

-   Boost.Beast HTTP server
-   C++20 coroutines (`co_await`, `co_spawn`)
-   Executor-aware request lifecycle
-   Controller → Service → Repository separation
-   Dependency Injection container
-   Structured logging
-   `.env` configuration

------------------------------------------------------------------------

## Concurrency Model

-   `boost::asio::awaitable<>`
-   Explicit executor ownership
-   Dedicated `thread_pool` for CPU-bound tasks
-   Custom `async_offload` abstraction
-   Deterministic lifetime handling
-   ASan + UBSan verified (GCC 14+)

### Stability Note

A GCC 11 coroutine codegen bug caused invalid frees due to lambda
capture copying inside coroutine frames.\
Upgrading to GCC 14 resolved the issue.\
Architecture validated under sanitizers without suppressions.

------------------------------------------------------------------------
### Logging

- Structured logging

- LoggerFactory-based logger creation

- Context-aware log entries (repository/service level)

- Centralized logger configuration

- Executor-safe usage (no hidden globals)

- Designed for extensibility (JSON sinks, file sinks, external exporters)

Logging is intentionally explicit.
No implicit global logger state is required by business logic.

------------------------------------------------------------------------
### API Documentation (Swagger / OpenAPI)

- Swagger/OpenAPI generation support

- Controller-level documentation mapping

- Auto-exposed API schema endpoint

- Designed to stay in sync with route definitions

- Suitable for client SDK generation

OpenAPI generation is part of the public API surface design, not an afterthought.

------------------------------------------------------------------------

## Database Layer

-   PostgreSQL (`libpq`)
-   Coroutine-friendly async wrapper
-   Connection pool abstraction
-   Transaction abstraction
-   Custom SQLBuilder
-   Optional-field update logic
-   Repository pattern

------------------------------------------------------------------------

## Authentication

-   JWT-based stateless auth
-   Password hashing:
    -   BCrypt
    -   libsodium (Argon2id)
-   CPU hashing offloaded to blocking thread pool
-   Rehash detection support

------------------------------------------------------------------------

## File System Service

-   File validation
-   MIME detection
-   Hashing
-   Controlled media path
-   Docker volume mount
-   Static serving via Nginx

------------------------------------------------------------------------

## Redis (future)

-   Redis connectivity integration planned
-   Cache abstraction layer planned (next milestone)

------------------------------------------------------------------------

# Testing

## Current State

| Type              | Status                |
|-------------------|-----------------------|
| Unit tests        | ❌ Not implemented yet |
| Integration tests | ⚠️ Partial            |
| E2E tests         | ✅ Implemented         |

Unit tests are planned.\
Current reliability is validated through full Docker-based E2E
environment.

------------------------------------------------------------------------

## E2E Architecture

    Client → Nginx → App → PostgreSQL
                        → Redis
                        → /media (Docker volume)

------------------------------------------------------------------------

## Running E2E

``` bash
make e2e-test
```

------------------------------------------------------------------------

## Media in E2E

`TEST_MEDIA` goes into `/work/media` inside Docker container.

Example:

``` yaml
volumes:
  - ./media:/work/media
```

Uploaded test files are written into container media directory and
served via Nginx.

------------------------------------------------------------------------

# Docker Support

Supports:

-   Development profile
-   E2E profile
-   Dedicated migration service
-   Media volume mounting
-   Custom Nginx config

------------------------------------------------------------------------

# Project Structure

    src/
     ├── controllers/
     ├── services/
     ├── repositories/
     ├── core/
     │    ├── db/
     │    ├── caching/
     │    ├── file_system/
     │    ├── filters/
     │    ├── configs/
     │    └── bootstrap.cpp
     └── main.cpp

------------------------------------------------------------------------

# Tech Stack

Layer     Technology
  --------- -------------------------
HTTP      Boost.Beast
Async     Boost.Asio (coroutines)
DB        PostgreSQL (libpq)
Cache     Redis
Hashing   BCrypt / libsodium
Build     CMake
Testing   Docker-based E2E
Runtime   Docker

------------------------------------------------------------------------

# Requirements

-   GCC 14+
-   C++20
-   CMake 3.20+
-   Docker (for E2E)
-   PostgreSQL 14+
-   Redis 7+

------------------------------------------------------------------------

# Known Limitations

-   No unit tests yet
-   No rate limiting
-   No cache abstraction layer
-   No OpenTelemetry

------------------------------------------------------------------------

# Roadmap Toward 1.0

-   Unit test layer (GoogleTest)
-   Redis cache connector abstraction
-   Swagger/OpenAPI generation
-   Graceful shutdown refinement

------------------------------------------------------------------------

# Status

v0.9.0 is a stable architectural baseline.

Core async model considered stable.\
Memory verified under sanitizers.\
Suitable for production experimentation and internal services.

------------------------------------------------------------------------

# Credits

## Author

**Emilian (Emil) Erofeevskiy**  
Senior Software Engineer

Architecture, async model design, concurrency invariants, DI container, database layer, Docker environment, E2E pipeline and overall system direction.

This framework is built as a long-term architectural foundation, not a demo server.

---

## AI Collaboration

Parts of architectural discussion, code reviews, formatting and documentation refinement were assisted by ChatGPT.

AI was used as:
- reasoning partner
- edge-case challenger
- documentation assistant
- architectural discussion tool

Final design decisions, invariants and implementation direction remain author-driven.

---

# Afterword

This project is not an attempt to compete with high-level frameworks.

It is an attempt to:

- push deterministic async execution in C++
- remove hidden concurrency behavior
- enforce explicit CPU/IO separation
- build a stable architectural core

Version 0.9.0 is not the finish line.  
It is a structural checkpoint.

The goal is simple:

Build systems that behave exactly as designed — even under pressure.

If that matters to you, you’re in the right place.

------------------------------------------------------------------------

# Feedback & Contributions

I encourage you to review the code critically.

If you find architectural weaknesses, concurrency risks, performance issues, or design inconsistencies — open an issue and describe them in detail.

Constructive criticism is welcome.

This project aims for structural correctness and deterministic behavior.  
If something violates that principle, it should be discussed openly.

Clear reasoning > silent approval.
