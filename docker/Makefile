
ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
	detected_OS := Windows
else
	detected_OS := $(shell uname -s)
endif

ifeq ($(detected_OS),Windows)
	HOST_OS = windows
endif
ifeq ($(detected_OS),Darwin)
	HOST_OS = darwin
endif
ifeq ($(detected_OS),Linux)
	HOST_OS = linux
endif

REPO = docker-freertos-sdl2
YML = $(REPO)/docker-compose.yml
COMPOSE := $(shell command -v docker-compose 2> /dev/null)
EMULATOR := $(HOST_OS)
EXECUTABLE = FreeRTOS_Emulator
DIRECTORY := $(strip $(shell "dirname" $(realpath $(lastword $(MAKEFILE_LIST)))))

ifeq ($(HOST_OS),windows)
	EXPORT := export PROJECT_DIR='$(DIRECTORY)/..'
	PREPARE :=
	DESTROY :=
endif
ifeq ($(HOST_OS),darwin)
	EXPORT := export PROJECT_DIR='$(DIRECTORY)/..'
	PREPARE := xhost + 127.0.0.1
	DESTROY := xhost - 127.0.0.1
endif
ifeq ($(HOST_OS),linux)
	EXPORT := export PROJECT_DIR='$(DIRECTORY)/..'
	PREPARE := xhost +local:root
	DESTROY := xhost -local:root
endif

.PHONY: run
run: compile prepare kill
	@echo "Running Executable in Container..."
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) exec $(EMULATOR) bash -c "cd /tmp/project/bin ; ./$(EXECUTABLE)"
	@echo Finished with RC=$$?
	@$(DESTROY)

PHONY: prepare
prepare:
	@$(PREPARE)

.PHONY: docker
docker:
	@echo "Looking for Docker installation..."
ifndef COMPOSE
	$(error "docker-compose not found. Please install Docker-Desktop before running make!")
endif
	@echo "Found: $(COMPOSE)"

.PHONY: up
up: docker
	@$(eval COUNT=$(shell $(EXPORT) ; '$(COMPOSE)' -f $(YML) ps | grep '$(REPO)_$(EMULATOR)' | wc -l))
	@test $(COUNT) -eq 0 && (echo "Creating Container..." ; $(EXPORT) ;'$(COMPOSE)' -f $(YML) up -d $(EMULATOR)) || :
	@test $(COUNT) -ne 2 && : || (echo "More than 1 Container is running. Please shut them down!"; false)


.PHONY: start
start: up
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) ps | grep '$(REPO)_$(EMULATOR)' | grep -q 'Up' || (echo "Starting Container..." ; $(EXPORT) ; '$(COMPOSE)' -f $(YML) start $(EMULATOR))

.PHONY: compile
compile:: start
	@echo "Compiling..."
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) exec $(EMULATOR) bash -c "cd /tmp/build && make"
	@echo Finished with RC=$$?

.PHONY: debug
debug: compile prepare
	@echo "Starting Debug Session..."
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) exec $(EMULATOR) bash -c "cd /tmp/project/bin && gdb -q -n -x .gdbinit -ex 'break main' -ex 'winheight src 20' -ex 'run' $(EXECUTABLE)"
	@echo Finished with RC=$$?
	@$(DESTROY)

.PHONY: clean
clean: start
	@echo "Cleaning up..."
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) exec $(EMULATOR) bash -c "cd /tmp/build && make clean"

.PHONY: shell
shell: start
	@echo "Connecting to a shell..."
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) exec $(EMULATOR) bash

.PHONY: stop
stop: docker
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) ps | grep '$(REPO)_$(EMULATOR)' | grep -q 'Up' && (echo "Stopping Container..." ; $(EXPORT) ; '$(COMPOSE)' -f $(YML) stop $(EMULATOR)) || :

.PHONY: reload
reload: docker
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) ps | grep '$(REPO)_$(EMULATOR)' | grep -q 'Up' && (echo "Restarting Container..." ; $(EXPORT) ; '$(COMPOSE)' -f $(YML) restart $(EMULATOR)) || :
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) ps | grep '$(REPO)_$(EMULATOR)' | grep -q 'Exit' && (echo "Starting Container..." ; $(EXPORT) ; '$(COMPOSE)' -f $(YML) start $(EMULATOR)) || :


.PHONY: down
down: stop
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) ps | grep -q '$(REPO)_$(EMULATOR)' && (echo "Removing Containers..." ; $(EXPORT) ; '$(COMPOSE)' -f $(YML) down) || :

.PHONY: cmake
cmake: start
	@echo "(Re)-Building CMake Configuration"
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) exec $(EMULATOR) bash -c "cd /tmp/build && cmake ../project"

.PHONY: gdbserver
gdbserver: start kill prepare
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) exec -d $(EMULATOR) bash -c "cd /tmp/project/bin && gdbserver localhost:2000 $(EXECUTABLE)"

.PHONY: kill
kill: start
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) exec $(EMULATOR) bash -c "killall -q -9 $(EXECUTABLE) || : ; killall -q -9 gdb || : ; killall -q -9 gdbserver || :"

.PHONY: logs
logs: docker
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) logs $(EMULATOR)

.PHONY: tidy
tidy: start
	@$(EXPORT) ; '$(COMPOSE)' -f $(YML) exec $(EMULATOR) bash -c "cd /tmp/build && make tidy"
