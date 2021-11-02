package main

import (
	rl "github.com/gen2brain/raylib-go/raylib"

	"github.com/jacobrec/there-is-no-bug/game"
	"runtime"
)

const screenWidth = 800
const screenHeight = 450

func main() {

	runtime.LockOSThread()

	rl.InitWindow(screenWidth, screenHeight, "there is no bug")
	defer rl.CloseWindow()

	rl.SetTargetFPS(60)

	idx := 0

	for {
		idx = game.EditorRun(idx)
		if idx == -1 {
			return
		}
		game.RunGame()

		// TODO: run game
	}

}
