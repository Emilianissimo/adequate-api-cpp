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
	docker compose --env-file .env.test -f docker-compose.e2e.yaml run --rm test_e2e bash -lc "cmake -S . -B build -G Ninja -DENABLE_TESTS=ON && cmake --build build -j --target e2e_tests"

e2e-test-count:
	docker compose --env-file .env.test -f docker-compose.e2e.yaml run --rm test_e2e bash -lc "ctest --test-dir build -N"

e2e-test:
	docker compose --env-file .env.test -f docker-compose.e2e.yaml up -d --build test_db test_redis test_migrate test_app test_nginx
	docker compose --env-file .env.test -f docker-compose.e2e.yaml run --rm test_e2e
	docker compose --env-file .env.test -f docker-compose.e2e.yaml down -v --remove-orphans
