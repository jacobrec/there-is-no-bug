package game

import (
	rl "github.com/gen2brain/raylib-go/raylib"
)

type Screen interface {
	Init()
	Draw()
	Update(float32)
}

func LoadMapFromIndex(idx int) Map {
	return LoadMap(loadMapList()[idx])
}

func RunGame() {
	screen := LoadGameFromMap(LoadMapFromIndex(0))

	lastTime := rl.GetTime()
	screen.Init()
	for !rl.WindowShouldClose() {
		delta := rl.GetTime() - lastTime
		lastTime += delta
		screen.Update(delta)
		screen.Draw()
	}

}

type Game struct {
	entities []Entity
}

type Entity interface {
	Update(float32)
	Draw()
}
type Block struct {
	position rl.Vector2
	size     int
}

func (b *Block) Update(delta float32) {}
func (b *Block) Draw() {
	rl.DrawRectangle(int32(b.position.X), int32(b.position.Y), int32(b.size), int32(b.size), rl.Black)
}

type Ladder struct {
	position rl.Vector2
	size     int
}

func (b *Ladder) Update(delta float32) {}
func (b *Ladder) Draw() {
	rl.DrawRectangle(int32(b.position.X), int32(b.position.Y), int32(b.size), int32(b.size), rl.Gray)
}

type Player struct {
	g            *Game
	position     rl.Vector2
	velocity     rl.Vector2
	size         int
	grounded     bool
	wallslide    int
	jumpcooldown float32
}

func (p *Player) Update(delta float32) {
	p.jumpcooldown -= delta
	pMass := float32(1)
	gravity := rl.NewVector2(0, 5000)
	forces := make([]rl.Vector2, 1)
	forces[0] = gravity

	acc := rl.NewVector2(0, 0)
	for _, f := range forces {
		acc = rl.Vector2Add(acc, rl.Vector2Scale(f, 1/pMass))
	}

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
	_ = keyDown

	const speed = 300
	if keyLeft && !keyRight {
		p.velocity.X -= 3 * speed * delta
		if p.velocity.X < -speed {
			p.velocity.X = -speed
		}
	} else if keyRight && !keyLeft {
		p.velocity.X += 3 * speed * delta
		if p.velocity.X > speed {
			p.velocity.X = speed
		}
	}

	p.grounded = false
	p.wallslide = 0

	climbing := false
	// Collisions
	for _, e := range p.g.entities {
		switch v := e.(type) {
		case *Ladder:
			bbe := GetBoundingBox2DFromSquare(v.position, float32(v.size))
			bbp := GetBoundingBox2DFromSquare(p.position, float32(p.size))
			if rl.CheckCollisionBoxes(bbe, bbp) {
				climbing = true
				p.velocity.Y = 0
				p.velocity.X = 0
			}
		case *Block:
			bbe := GetBoundingBox2DFromSquare(v.position, float32(v.size))
			dOff := float32(5)
			pb := rl.NewBoundingBox(rl.NewVector3(p.position.X+dOff, p.position.Y+float32(p.size/2), 0),
				rl.NewVector3(p.position.X+float32(p.size)-dOff, p.position.Y+float32(p.size), 1))
			pt := rl.NewBoundingBox(rl.NewVector3(p.position.X+dOff, p.position.Y, 0),
				rl.NewVector3(p.position.X+float32(p.size)-dOff, p.position.Y+float32(p.size/2), 1))
			pr := rl.NewBoundingBox(rl.NewVector3(p.position.X+dOff, p.position.Y, 0),
				rl.NewVector3(p.position.X+float32(p.size), p.position.Y+float32(p.size)-dOff, 1))
			pl := rl.NewBoundingBox(rl.NewVector3(p.position.X, p.position.Y+dOff, 0),
				rl.NewVector3(p.position.X+float32(p.size)-dOff, p.position.Y+float32(p.size)-dOff, 1))
			if rl.CheckCollisionBoxes(bbe, pb) {
				// Collision Bottom
				p.position.Y = v.position.Y - float32(p.size)
				p.velocity.Y = 0
				p.grounded = true
			} else if rl.CheckCollisionBoxes(bbe, pt) {
				// Collision top
				p.position.Y = v.position.Y + float32(v.size)
				p.velocity.Y = 0
			} else if rl.CheckCollisionBoxes(bbe, pr) {
				// Collision Right
				p.position.X = v.position.X - float32(p.size)
				p.wallslide = -1
				if p.velocity.X > 0 {
					p.velocity.X = 0
				}
			} else if rl.CheckCollisionBoxes(bbe, pl) {
				// Collision Left
				p.position.X = v.position.X + float32(p.size)
				p.wallslide = 1
				if p.velocity.X < 0 {
					p.velocity.X = 0
				}
			}
		}
	}

	if climbing {
		const climbSpeed = 150
		if !keyLeft && keyRight {
			p.position.X += climbSpeed * delta
		} else if !keyRight && keyLeft {
			p.position.X -= climbSpeed * delta
		}
		if !keyDown && keyJump {
			p.position.Y -= climbSpeed * delta
		} else if !keyJump && keyDown {
			p.position.Y += climbSpeed * delta
		}
	} else {
		const jumpCooldown = 0.2
		if keyJump && p.jumpcooldown < 0 {
			if p.grounded {
				p.velocity.Y -= 1000
				p.jumpcooldown = jumpCooldown
			} else if p.wallslide != 0 {
				p.velocity.Y -= 800
				p.velocity.X = float32(p.wallslide * 500)
				p.jumpcooldown = jumpCooldown
			}
		}

		p.velocity = rl.Vector2Add(p.velocity, rl.Vector2Scale(acc, delta))
		p.position = rl.Vector2Add(p.position, rl.Vector2Scale(p.velocity, delta))
		p.velocity = rl.Vector2Scale(p.velocity, 0.99) // friction

		if p.grounded && !keyLeft && !keyRight {
			p.velocity.X *= 0.5
		}
	}

}
func GetBoundingBox2DFromSquare(pointBL rl.Vector2, size float32) rl.BoundingBox {
	return rl.NewBoundingBox(rl.NewVector3(pointBL.X, pointBL.Y, 0), rl.NewVector3(pointBL.X+size, pointBL.Y+size, 1))
}

func (p *Player) Draw() {
	c := rl.Orange
	if p.wallslide != 0 {
		c = rl.Red
	}
	rl.DrawRectangle(int32(p.position.X), int32(p.position.Y), int32(p.size), int32(p.size), c)
}

func LoadGameFromMap(m Map) Game {
	entities := make([]Entity, 0)
	const scale = 4
	const tilesize = 8
	p := Player{}
	g := Game{entities}
	for i, v := range m.Tiledata {
		x := scale * (i % m.Width) * tilesize
		y := scale * (i / m.Width) * tilesize
		if v == 1 {
			e := Block{
				position: rl.NewVector2(float32(x), float32(y)),
				size:     tileSize * scale,
			}
			entities = append(entities, &e)
		} else if v == 2 {
			e := Ladder{
				position: rl.NewVector2(float32(x), float32(y)),
				size:     tileSize * scale,
			}
			entities = append(entities, &e)
		} else if v == 3 {
			p = Player{
				position: rl.NewVector2(float32(x), float32(y)),
				size:     tileSize * scale,
				g:        &g,
			}
		}
	}
	entities = append(entities, &p)
	g.entities = entities
	return g
}

func (g *Game) Init() {}
func (g *Game) Update(delta float32) {
	for _, e := range g.entities {
		e.Update(delta)
	}
}
func (g *Game) Draw() {
	rl.BeginDrawing()
	rl.ClearBackground(rl.RayWhite)
	for _, e := range g.entities {
		e.Draw()
	}
	rl.EndDrawing()
}
