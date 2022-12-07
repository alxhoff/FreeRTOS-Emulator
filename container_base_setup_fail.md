# Container Toolchain Setup

## 1. Installing  Docker on your platform

* Refer to the link to install for your prefered Operating System <https://docs.docker.com/desktop/install/mac-install/>

## 2. Setting Up the Toolchain
* ## A. Configuring Docker for Ease of Use

* ### Run Docker commands without sudo
This Apply to Linux base machines but `use sudo infront of the command if you love sudo a lot`

* Add the docker group if it doesn't already exist

```bash
$ sudo groupadd docker
```

* Add the connected user $USER to the docker group
* Optionally change the username to match your preferred user.

```bash
$ sudo gpasswd -a $USER docker
```

* IMPORTANT: Log out and log back in so that your group membership is re-evaluated.

* Restart the docker daemon

```bash
$ sudo service docker restart
```

* If you are on Ubuntu 14.04-15.10, use docker.io instead:

```bash
$ sudo service docker.io restart
```

* ### Still Geeting Permission Denied

```bash
$ sudo chmod 666 /var/run/docker.sock
```

* ## B. Running Docker using Ubuntu Image
* pulling Ubuntu Xenial image
```bash
$ docker image pull ubuntu:xenial
```
* list of current containers using

```bash
$ docker ps -a
```

* only see the running containers

```bash
$ docker ps
```

* last container that was run

```bash
$ docker ps -l
```

* Naming a container

```bash
$ docker container run  --net host -v /tmp/.X11-unix:/tmp/.X11-unix -dit --name free ubuntu:xenial
```

*This would create a new container free

* Starting a start or stopped free

```bash
$ docker start emulator
$ docker start emulator
```
* Launch the container
```bash
$ docker exec -it free bash
```
* ## C. Installing Packages in your new container
* Refresh your package manager
```bash
$ pwd
$ cd
$ pwd
$ apt update
```

* Installing  basic utilities like `make, cmake, git, vim`
```bash
$ apt install make cmake git vim 
```
* Install Depencies
```bash
$ apt install build-essential libsdl2-ttf-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-gfx-dev libsdl2-dev
```

* Additional Requirement
```bash
$ apt install clang-4.0 clang-tidy-4.0
```
* Generating SSH
```bash
ssh-keygen -t ed25519 -C "your_email@example.com" # use GitHub Email
```
* Copy the public key into your Github and Test it
* Refer here for more <https://docs.github.com/en/authentication/connecting-to-github-with-ssh/about-ssh>

```bash
$ cat .ssh/id_ed25519.pub 
$ ssh -T git@github.com
```
* Run this on your host machine
```bash
$ xhost +local:
```
* Run this on your container
```bash
$ export DISPLAY=:0
```
* clone the Project
```bash
$ git@github.com:GODINME/FreeRTOS-Emulator.git
```
* follow the doc to know to compile and run.