package main

import (
	phys "github.com/gen2brain/raylib-go/physics"
	gui "github.com/gen2brain/raylib-go/raygui"
	rl "github.com/gen2brain/raylib-go/raylib"

	"runtime"
	"unsafe"
)

const screenWidth = int32(800)
const screenHeight = int32(450)

func loop(gs *GameState) {

	keyLeft := rl.IsKeyDown(int32(rl.KeyLeft)) ||
		rl.IsKeyDown(int32(rl.KeyA)) ||
		rl.IsKeyDown(int32(rl.KeyH))
	keyRight := rl.IsKeyDown(int32(rl.KeyRight)) ||
		rl.IsKeyDown(int32(rl.KeyD)) ||
		rl.IsKeyDown(int32(rl.KeyL))
	keyJump := rl.IsKeyDown(int32(rl.KeySpace)) ||
		rl.IsKeyDown(int32(rl.KeyW)) ||
		rl.IsKeyDown(int32(rl.KeyK)) ||
		rl.IsKeyDown(int32(rl.KeyUp))
	keyDown := rl.IsKeyDown(int32(rl.KeyS)) ||
		rl.IsKeyDown(int32(rl.KeyJ)) ||
		rl.IsKeyDown(int32(rl.KeyDown))

	shouldLeft := keyLeft && !keyRight
	shouldRight := keyRight && !keyLeft
	shouldJump := keyJump && !keyDown

	moveForce := float32(0.3)
	if shouldLeft {
		gs.player.Velocity.X = -moveForce
	} else if shouldRight {
		gs.player.Velocity.X = moveForce
	}
	if shouldJump && gs.player.IsGrounded {
		gs.player.Velocity.Y = 5 * -moveForce
	}

	phys.Update()

	bodiesCount := phys.GetBodiesCount()
	for i := bodiesCount - 1; i >= 0; i-- {
		body := phys.GetBody(i)
		if body != nil && int32(body.Position.Y) > screenHeight*2 {
			if body == gs.player {
				gs.player.Enabled = false
			} else {
				phys.DestroyBody(body)
			}
		}
	}

	rl.BeginDrawing()

	rl.ClearBackground(rl.Black)

	buttonClicked := gui.Button(rl.NewRectangle(10, 30, 50, 20), "Reset")
	_ = buttonClicked
	if rl.IsKeyPressed(int32(rl.KeyR)) || buttonClicked {
		gs.player.Enabled = true
		gs.player.Position = rl.NewVector2(float32(screenWidth/2), float32(screenHeight/2))
		gs.player.Velocity = rl.NewVector2(0, 0)
	}

	rl.DrawFPS(screenWidth-90, screenHeight-30)

	bodiesCount = phys.GetBodiesCount()
	for i := 0; i < bodiesCount; i++ {
		body := phys.GetBody(i)

		if body != nil {
			vertexCount := phys.GetShapeVerticesCount(i)
			for j := 0; j < vertexCount; j++ {
				vertexA := body.GetShapeVertex(j)

				var jj int
				if j+1 < vertexCount {
					jj = j + 1
				} else {
					jj = 0
				}

				vertexB := body.GetShapeVertex(jj)

				rl.DrawLineV(
					*(*rl.Vector2)(unsafe.Pointer(&vertexA)),
					*(*rl.Vector2)(unsafe.Pointer(&vertexB)),
					rl.Green,
				)
			}
		}
	}

	rl.DrawText("Press 'Esc' to quit", 10, 10, 10, rl.White)

	rl.EndDrawing()
}

type GameState struct {
	floor  []*phys.Body
	player *phys.Body
}

func makeFloor() []*phys.Body {
	var flooring [3]*phys.Body
	floor := phys.NewBodyRectangle(
		rl.NewVector2(float32(screenWidth/2), float32(screenHeight)),
		500,
		100,
		10,
	)
	floor.Enabled = false

	platform1 := phys.NewBodyRectangle(
		rl.NewVector2(400, 320),
		float32(screenWidth/2),
		30,
		10,
	)
	platform1.Enabled = false

	platform2 := phys.NewBodyRectangle(
		rl.NewVector2(500, 220),
		30,
		150,
		10,
	)
	platform2.Enabled = false

	flooring[0] = floor
	flooring[1] = platform1
	flooring[2] = platform2

	return flooring[:]
}

func MakePlayer() *phys.Body {
	player := phys.NewBodyRectangle(
		rl.NewVector2(float32(screenWidth/2), float32(screenHeight/2)),
		50,
		50,
		10,
	)
	player.FreezeOrient = true
	player.Enabled = true
	return player
}

func main() {

	runtime.LockOSThread()

	rl.InitWindow(screenWidth, screenHeight, "[there is no bug] - Simple Platform demo")
	defer rl.CloseWindow()

	phys.Init()

	rl.SetTargetFPS(60)
	gameState := GameState{
		floor:  makeFloor(),
		player: MakePlayer(),
	}

	for !rl.WindowShouldClose() {
		loop(&gameState)
	}

	phys.Close()
}
