build-app:
	docker-compose build app

build: build-app
	docker-compose build

start-app:
	docker-compose up -d app

start:
	docker-compose up -d

up:
	docker-compose up

stop:
	docker-compose stop

down:
	docker-compose down
