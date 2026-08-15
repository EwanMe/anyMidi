docker_image = anymidi-build

.PHONY: image
image:
	docker build -t $(docker_image) .

.PHONY: configure
configure:
	docker run -u $$UID:$$GID -v ./:/src:rw -w /src $(docker_image) cmake --preset release

.PHONY: build
build:
	docker run -u $$UID:$$GID -v ./:/src:rw -w /src $(docker_image) cmake --build --preset release