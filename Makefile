SHELL := /bin/sh

BACKEND_DIR := backend
DJANGO_DIR := backend/django
PYTHON_SIM_DIR := device/targets/python-sim

COMPOSE := docker compose

STM32_DIR := device/targets/stm32
STM32_BUILD_DIR := $(STM32_DIR)/build/debug
STM32_ELF := $(STM32_BUILD_DIR)/tilekoumanto_stm32.elf
STM32_BIN := $(STM32_BUILD_DIR)/tilekoumanto_stm32.bin

.PHONY: help
help:
	@echo "Tilekoumanto project commands"
	@echo ""
	@echo "Stack:"
	@echo "  make stack-up            Start local Docker stack in background"
	@echo "  make stack-down          Stop local Docker stack"
	@echo "  make stack-restart       Restart local Docker stack"
	@echo "  make stack-logs          Follow all stack logs"
	@echo "  make logs-web            Follow django-web logs"
	@echo "  make logs-mqtt           Follow django-mqtt-catcher logs"
	@echo "  make logs-mosquitto      Follow mosquitto logs"
	@echo "  make logs-postgres       Follow postgres logs"
	@echo "  make logs-caddy          Follow caddy logs"
	@echo ""
	@echo "Backend:"
	@echo "  make backend-test        Run Django tests locally"
	@echo "  make backend-migrate     Run Django migrations inside django-web"
	@echo "  make backend-shell       Open shell inside django-web container"
	@echo ""
	@echo "Python simulator:"
	@echo "  make sim-build           Configure and build python-sim C core"
	@echo "  make sim-run             Run python simulator locally"
	@echo "  make sim-test            Run python simulator pytest tests"
	@echo ""
	@echo "C core:"
	@echo "  make c-test              Run CTest for device core logic"
	@echo ""
	@echo "All:"
	@echo "  make test                Run backend, simulator, and C tests"
	@echo "  make clean               Remove local build/cache artifacts"
	@echo ""
	@echo ""
	@echo "STM32:"
	@echo "  make stm32-build         Configure and build standalone STM32 firmware"
	@echo "  make stm32-flash         Flash standalone STM32 firmware with OpenOCD"
	@echo "  make stm32-clean         Remove STM32 build artifacts"

.PHONY: stack-up
stack-up:
	cd $(BACKEND_DIR) && $(COMPOSE) up -d

.PHONY: stack-down
stack-down:
	cd $(BACKEND_DIR) && $(COMPOSE) down

.PHONY: stack-restart
stack-restart:
	cd $(BACKEND_DIR) && $(COMPOSE) down
	cd $(BACKEND_DIR) && $(COMPOSE) up -d

.PHONY: stack-logs
stack-logs:
	cd $(BACKEND_DIR) && $(COMPOSE) logs --tail=100 -f

.PHONY: logs-web
logs-web:
	cd $(BACKEND_DIR) && $(COMPOSE) logs --tail=100 -f django-web

.PHONY: logs-mqtt
logs-mqtt:
	cd $(BACKEND_DIR) && $(COMPOSE) logs --tail=100 -f django-mqtt-catcher

.PHONY: logs-mosquitto
logs-mosquitto:
	cd $(BACKEND_DIR) && $(COMPOSE) logs --tail=100 -f mosquitto

.PHONY: logs-postgres
logs-postgres:
	cd $(BACKEND_DIR) && $(COMPOSE) logs --tail=100 -f postgres

.PHONY: logs-caddy
logs-caddy:
	cd $(BACKEND_DIR) && $(COMPOSE) logs --tail=100 -f caddy

.PHONY: backend-test
backend-test:
	cd $(DJANGO_DIR) && $(COMPOSE) exec django-web python manage.py test devices

.PHONY: backend-migrate
backend-migrate:
	cd $(BACKEND_DIR) && $(COMPOSE) exec django-web python manage.py migrate

.PHONY: backend-shell
backend-shell:
	cd $(BACKEND_DIR) && $(COMPOSE) exec django-web sh

.PHONY: sim-build
sim-build:
	cd $(PYTHON_SIM_DIR) && cmake -S . -B build/debug -DBUILD_TESTING=ON
	cd $(PYTHON_SIM_DIR) && cmake --build build/debug

.PHONY: sim-run
sim-run:
	cd $(PYTHON_SIM_DIR) && python -m app.main

.PHONY: sim-test
sim-test:
	cd $(PYTHON_SIM_DIR) && python -m pytest

.PHONY: c-test
c-test: sim-build
	cd $(PYTHON_SIM_DIR) && ctest --test-dir build/debug/core --output-on-failure

.PHONY: test
test: backend-test sim-test c-test

.PHONY: clean
clean:
	rm -rf $(PYTHON_SIM_DIR)/build
	rm -rf $(STM32_DIR)/build
	find . -type d -name "__pycache__" -prune -exec rm -rf {} +
	find . -type d -name ".pytest_cache" -prune -exec rm -rf {} +

.PHONY: stm32-build
stm32-build:
	cd $(STM32_DIR) && cmake -S . -B build/debug \
		-DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
	cd $(STM32_DIR) && cmake --build build/debug

.PHONY: stm32-flash
stm32-flash: stm32-build
	openocd \
		-f interface/stlink.cfg \
		-f target/stm32f4x.cfg \
		-c "program $(STM32_ELF) verify reset exit"

.PHONY: stm32-clean
stm32-clean:
	rm -rf $(STM32_DIR)/build