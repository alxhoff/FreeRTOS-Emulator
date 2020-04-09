
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

EXECUTABLE = FreeRTOS_Emulator
DIRECTORY := $(strip $(shell "dirname" $(realpath $(lastword $(MAKEFILE_LIST)))))

USER := rtos-sim
PASS := rtos-sim
SERVER := 192.168.57.106
SSH_CMD := ssh -o "StrictHostKeyChecking=no" -o "PasswordAuthentication=no" -i ssh/rtos-sim $(USER)@$(SERVER)

ifeq ($(detected_OS),Windows)
  # TODO
	TMP_DIR :=
	PING_CMD :=
	MOUNT_CMD :=
	COPY_CMD :=
	CLEANUP :=
endif
ifeq ($(detected_OS),Darwin)
	TMP_DIR := /tmp/emulator
	PING_CMD := ping -q -c 1 $(SERVER) > /dev/null && echo "Found VM!" || (echo "VM not found!" ; exit 1)
	MOUNT_CMD := smbutil statshares -a | grep -q $(SERVER) && echo "Already mounted!" || ((mkdir -p $(TMP_DIR)/share && mount_smbfs //$(USER):$(PASS)@$(SERVER)/Share $(TMP_DIR)/share) || (echo "Failed to mount!" ; exit 1) && echo "Mounted!")
	COPY_CMD := (rm -rf $(TMP_DIR)/share/* && cp -R $(DIRECTORY)/../* $(TMP_DIR)/share/) && echo "Successful!" || (echo "Failed!" ; exit 1)
	CLEANUP := smbutil statshares -a | grep -q $(SERVER) && (echo "Disconnect!" ; umount $(TMP_DIR)/share) || echo "Nothing to clean!"
endif
ifeq ($(detected_OS),Linux)
	# TODO
	TMP_DIR := /tmp/emulator/
	PING_CMD := ping -q -c 1 $(SERVER) > /dev/null && echo "Found VM!" || (echo "VM not found!" ; exit 1)
	MOUNT_CMD := mkdir -p $(TMP_DIR)/share && sudo mount -t cifs //$(SERVER)/Share -o rw -o user=$(USER) -o password=$(PASS)
	COPY_CMD := rm -rf $(TMP_DIR)/share/* && cp -r -d $(DIRECTORY)/../* $(TMP_DIR)/share/
	CLEANUP := mount -t cifs | grep -q $(SERVER) && (echo "Disconnect!" ; umount $(TMP_DIR)/share) || echo "Nothing to clean!"
endif

.PHONY: run
run: compile prepare
	@echo "Running executable..."
	@$(SSH_CMD) "bash -c 'cd /tmp/project/bin/ && DISPLAY=:0 ./$(EXECUTABLE)'"
	@echo Finished with RC=$$?

.PHONY: debug
debug: compile prepare
	@echo "Starting Debug Session..."
	@$(SSH_CMD) "bash -c \"cd /tmp/project/bin && gdb -q -n -x .gdbinit -ex 'set environment DISPLAY = :0' -ex 'break main' -ex 'winheight src 20' -ex 'run' $(EXECUTABLE)\""
	@echo Finished with RC=$$?

.PHONY: clean
clean: cmake
	@echo "Cleaning up..."
	@$(SSH_CMD) "bash -c 'cd /tmp/build && make clean'"

.PHONY: shell
shell: ping
	@echo "Connecting to a shell..."
	@$(SSH_CMD)

.PHONY: gdbserver
gdbserver: prepare
	@echo "Setting up gdbserver..."
	@$(SSH_CMD) "bash -c 'cd /tmp/project/bin && gdbserver localhost:2000 $(EXECUTABLE)'"

.PHONY: kill
kill: ping
	@$(SSH_CMD) "bash -c 'killall -q -9 $(EXECUTABLE) || : ; killall -q -9 gdb || : ; killall -q -9 gdbserver || :'"

.PHONY: ping
ping:
	@echo "Looking for VM..."
	@$(PING_CMD)
	@test -f ssh/rtos-sim || (echo "Creating SSH Keys..." ; ssh-keygen -t rsa -C 'rtos-sim' -N '' -f ssh/rtos-sim -q)
	@$(SSH_CMD) exit || (echo "Copying public key to VM... [PLEASE ENTER THE PASSWORD '$(PASS)' IF PROMPTED!]" ;  ssh-copy-id -i ssh/rtos-sim $(USER)@$(SERVER))

.PHONY: mount
mount: ping
	@echo "Mounting shared directory..."
	@$(MOUNT_CMD)

.PHONY: sync
sync: ping mount
	@echo "Syncing project directory..."
	@$(COPY_CMD)

.PHONY: quit
quit:
	@echo "Cleaning up..."
	$(CLEANUP)

.PHONY: cmake
cmake: sync
	@echo "(Re)-Building CMake Configuration"
	@$(SSH_CMD) "bash -c 'rm -rf /tmp/build && mkdir -p /tmp/build && cd /tmp/build && cmake -DENABLE_CLANG_TIDY=ON /tmp/project'"

.PHONY: compile
compile: cmake
	@echo "Compiling..."
	@$(SSH_CMD) "bash -c 'cd /tmp/build && make'"
	@echo Finished with RC=$$?

.PHONY: prepare
prepare: ping
	@echo "Preparing VM..."
	@$(SSH_CMD) "bash -c 'DISPLAY=:0 /opt/espl/screen_zoom.sh'"
	@echo "OK"

.PHONY: tidy
tidy: sync
	@$(SSH_CMD) "bash -c 'cd /tmp/build && make tidy'"
