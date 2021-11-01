package main

import (
	phys "github.com/chunqian/go-raylib/physac"
	rl "github.com/chunqian/go-raylib/raylib"

	"fmt"
	"runtime"
	"unsafe"
)

const screenWidth = int32(800)
const screenHeight = int32(450)

func v2(x, y float32) phys.Vector2 {
	return phys.NewVector2(x, y)
}

func loop(gs *GameState) {

	keyLeft := rl.IsKeyDown(int32(rl.KEY_LEFT)) ||
		rl.IsKeyDown(int32(rl.KEY_A)) ||
		rl.IsKeyDown(int32(rl.KEY_H))
	keyRight := rl.IsKeyDown(int32(rl.KEY_RIGHT)) ||
		rl.IsKeyDown(int32(rl.KEY_D)) ||
		rl.IsKeyDown(int32(rl.KEY_L))
	keyJump := rl.IsKeyDown(int32(rl.KEY_SPACE)) ||
		rl.IsKeyDown(int32(rl.KEY_W)) ||
		rl.IsKeyDown(int32(rl.KEY_K)) ||
		rl.IsKeyDown(int32(rl.KEY_UP))
	keyDown := rl.IsKeyDown(int32(rl.KEY_S)) ||
		rl.IsKeyDown(int32(rl.KEY_J)) ||
		rl.IsKeyDown(int32(rl.KEY_DOWN))

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

	phys.RunPhysicsStep()

	if rl.IsKeyPressed(int32(rl.KEY_R)) {
		gs.player.Enabled = true
		gs.player.Position = v2(float32(screenWidth/2), float32(screenHeight/2))
		gs.player.Velocity = v2(0, 0)
	}

	bodiesCount := phys.GetPhysicsBodiesCount()
	for i := bodiesCount - 1; i >= 0; i-- {
		body := phys.GetPhysicsBody(i)
		if body != nil && int32(body.Position.Y) > screenHeight*2 {
			if body == gs.player {
				gs.player.Enabled = false
			} else {
				phys.DestroyPhysicsBody(body)
			}
		}
	}

	rl.BeginDrawing()

	rl.ClearBackground(rl.Black)

	rl.DrawFPS(screenWidth-90, screenHeight-30)

	bodiesCount = phys.GetPhysicsBodiesCount()
	rl.DrawText(fmt.Sprintf("There are currently %v bodies", bodiesCount), 10, 40, 10, rl.White)
	rl.DrawText(fmt.Sprintf("player on ground: %v", gs.player.IsGrounded), 10, 55, 10, rl.White)
	for i := int32(0); i < bodiesCount; i++ {
		body := phys.GetPhysicsBody(i)

		if body != nil {
			vertexCount := phys.GetPhysicsShapeVerticesCount(i)
			for j := int32(0); j < vertexCount; j++ {
				vertexA := phys.GetPhysicsShapeVertex(body, j)

				var jj int32
				if j+1 < vertexCount {
					jj = j + 1
				} else {
					jj = 0
				}

				vertexB := phys.GetPhysicsShapeVertex(body, jj)

				rl.DrawLineV(
					*(*rl.Vector2)(unsafe.Pointer(&vertexA)),
					*(*rl.Vector2)(unsafe.Pointer(&vertexB)),
					rl.Green,
				)
			}
		}
	}

	rl.DrawText("Press 'R' to reset", 10, 10, 10, rl.White)
	rl.DrawText("Press 'Esc' to quit", 10, 25, 10, rl.White)

	rl.EndDrawing()
}

type GameState struct {
	floor  []*phys.PhysicsBodyData
	player *phys.PhysicsBodyData
}

func makePlayer() *phys.PhysicsBodyData {
	player := phys.CreatePhysicsBodyRectangle(
		phys.NewVector2(float32(screenWidth/2), float32(screenHeight/2)),
		50,
		50,
		10,
	)
	player.FreezeOrient = true
	player.Enabled = true
	return player
}

func makeFloor() []*phys.PhysicsBodyData {
	var flooring [3]*phys.PhysicsBodyData
	floor := phys.CreatePhysicsBodyRectangle(
		phys.NewVector2(float32(screenWidth/2), float32(screenHeight)),
		500,
		100,
		10,
	)
	floor.Enabled = false

	platform1 := phys.CreatePhysicsBodyRectangle(
		phys.NewVector2(400, 320),
		float32(screenWidth/2),
		30,
		10,
	)
	platform1.Enabled = false

	platform2 := phys.CreatePhysicsBodyRectangle(
		phys.NewVector2(500, 220),
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

func main() {

	runtime.LockOSThread()
	rl.SetConfigFlags(uint32(rl.FLAG_MSAA_4X_HINT))

	rl.InitWindow(screenWidth, screenHeight, "[there is no bug] - Simple Platform demo")
	defer rl.CloseWindow()

	phys.InitPhysics()

	rl.SetTargetFPS(60)
	gameState := GameState{
		floor:  makeFloor(),
		player: makePlayer(),
	}

	for !rl.WindowShouldClose() {
		loop(&gameState)
	}

	phys.ClosePhysics()
}
