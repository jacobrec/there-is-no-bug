game/game:
	cd game && go build

run: game/game
	./game/game

.PHONY: run
