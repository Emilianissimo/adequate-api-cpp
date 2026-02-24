.PHONY: set-env

set-env:
	mkdir -p MEDIA TEST_MEDIA
	test -f .env || cp .env.example .env
	test -f .env.test || cp .env.test.example .env.test
	@echo Environment prepared.

build-app:
	docker compose build app

start-app:
	docker compose up -d app

rebuild-app: build-app start-app

build:
	docker compose build

start:
	docker compose up -d

up:
	docker compose up

stop:
	docker compose stop

down:
	docker compose down

migrate:
	docker compose run --rm migrate up

rollback:
	docker compose run --rm migrate migrate down -all

migrate-clean-all:
	docker compose run --rm migrate drop -f

enter_db:
	docker exec -it cpp_api_db_container psql --username=core_db_user --dbname=core_db

e2e-test-build:
	docker compose -p cpp_api_e2e --env-file .env.test -f docker-compose.e2e.yaml up -d --build test_db test_redis test_migrate test_app test_nginx

e2e-test-remove:
	docker compose -p cpp_api_e2e --env-file .env.test -f docker-compose.e2e.yaml down -v --remove-orphans

e2e-test: e2e-test-build
	# should run one after another, for windows should be pasted manually or use semicolon separator
	docker compose -p cpp_api_e2e --env-file .env.test -f docker-compose.e2e.yaml run --rm test_e2e
	e2e-test-remove

e2e-test-count:
	docker compose -p cpp_api_e2e --env-file .env.test -f docker-compose.e2e.yaml run --rm test_e2e bash -lc "ctest --test-dir build -N"
