SOURCES = $(shell find $(SRC_PATH) -name '*.go')
run:
	cd game && go run main.go


.PHONY: run
