.ONESHELL:
DEVICE_PATH := $(shell python3 find_device.py)

bootstrap: requirements.txt ## make bootstrap # Install python virtualenv and source it
	@python3 -mvenv .venv
	. .venv/bin/activate
	pip install -r requirements.txt

install: bootstrap
	cp code.py $(DEVICE_PATH)/code.py
	cp -rp cpy_rotary/ $(DEVICE_PATH)/cpy_rotary/
	circup install -r requirements.circup.txt

devinstall: bootstrap
	circup install --auto

clean:
	rm -rf .venv
