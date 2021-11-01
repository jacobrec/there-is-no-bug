SOURCES = $(shell find $(SRC_PATH) -name '*.go')
game/game: $(SOURCES)
	cd game && go build

run: game/game
	./game/game

.PHONY: run
