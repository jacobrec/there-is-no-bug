package game

import (
	gui "github.com/gen2brain/raylib-go/raygui"
	rl "github.com/gen2brain/raylib-go/raylib"

	"fmt"
	"io/ioutil"
	"log"
	"strconv"
	"strings"
)

const ScreenWidth = 800
const ScreenHeight = 450
const tilesetPath = "assets/art/tilesets/"
const padding = 10
const panelWidth = 130
const panelPadding = padding
const panelLX = ScreenWidth - (panelWidth + 2*panelPadding)
const panelX = panelLX + panelPadding
const topBarHeight = 50
const saveButtonWidth = 50

const tileSize = 8

func EditorRun(mIdx int) int {
	tilesets := loadTilesets()
	tilesetNames := namesFromTilesets(tilesets)
	tileset := LoadTileset(tilesetPath + tilesetNames[0] + "/")
	images := LoadTilesetImages(tileset)
	mapNames := loadMapList()
	activeMap := mIdx
	m := LoadMap(mapNames[activeMap])

	tileActive := 0
	tilesetsActive := 0
	mapZoom := int32(4)

	tileNames := m.Tileset.Tiles
	manageSpecials := func() {
		tileNames = m.Tileset.Tiles
		for i := 0; i < m.Specials; i++ {
			tileNames = append(tileNames, "Special "+strconv.Itoa(i))
		}
	}
	manageSpecials()

	fadedColor := rl.NewColor(50, 50, 50, 50)
	for !rl.WindowShouldClose() {
		rl.BeginDrawing()
		rl.ClearBackground(rl.RayWhite)
		mx := rl.GetMouseX()
		my := rl.GetMouseY()
		mv := rl.NewVector2(float32(mx), float32(my))

		// Map and Map Grid overlay
		for i := 0; i < len(m.Tiledata); i++ {
			x := int32(i % m.Width)
			y := int32(i / m.Width)
			tx := x * tileSize * mapZoom
			ty := topBarHeight + y*tileSize*mapZoom
			tw := mapZoom * tileSize
			th := mapZoom * tileSize
			drawTile := func(idx int) {
				if idx < len(images) {
					rl.DrawTextureEx(images[idx], rl.NewVector2(float32(tx), float32(ty)),
						0, float32(mapZoom), rl.White)
				} else if idx < len(images)+m.Specials {
					r := float32(tw) / 3
					rl.DrawEllipse(tx+tw/2, ty+th/2, r, r, rl.Orange)
					text := strconv.Itoa(idx - len(images))
					fontSize := int32(tileSize)
					rl.DrawText(text, tx+tw/2-rl.MeasureText(text, fontSize)/2, ty+th/2-tileSize/2, fontSize, rl.Gray)
				}
			}
			drawTile(m.Tiledata[i])
			rl.DrawRectangleLines(tx, ty, tw, th, rl.Gray)
			if rl.CheckCollisionPointRec(mv, rl.NewRectangle(float32(tx), float32(ty), float32(tw), float32(th))) {
				rl.DrawRectangle(tx, ty, tw, th, fadedColor)
				drawTile(tileActive)
				if rl.IsMouseButtonDown(rl.MouseLeftButton) {
					m.Tiledata[i] = tileActive
				}
			}

		}
		// Topbar
		activeMap = gui.ComboBox(rl.NewRectangle(padding, padding, 100, topBarHeight-2*padding), mapNames, activeMap)
		if gui.Button(rl.NewRectangle(padding+100+50, padding, 20, topBarHeight-2*padding), "Switch Map") {
			m = LoadMap(mapNames[activeMap])
		}

		if gui.Button(rl.NewRectangle(panelLX-padding-saveButtonWidth, padding, saveButtonWidth, topBarHeight-2*padding), "Save") {
			SaveMap(&m)
		}
		if gui.Button(rl.NewRectangle(panelLX-padding-3*saveButtonWidth, padding, saveButtonWidth, topBarHeight-2*padding), "Zoom +") {
			mapZoom++
		}
		if gui.Button(rl.NewRectangle(panelLX-padding-2*saveButtonWidth, padding, saveButtonWidth, topBarHeight-2*padding), "Zoom -") {
			mapZoom--
			if mapZoom < 1 {
				mapZoom = 1
			}
		}
		if gui.Button(rl.NewRectangle(panelLX-padding-4*saveButtonWidth, padding, saveButtonWidth, topBarHeight-2*padding), "Run") {
			return activeMap
		}
		rl.DrawLine(0, topBarHeight, panelLX, topBarHeight, rl.Gray)
		// Sidebar
		const labelHeight = 20
		rl.DrawRectangle(panelLX, 0, ScreenWidth, ScreenHeight, rl.RayWhite)
		gui.Label(rl.NewRectangle(panelX, padding, panelWidth, labelHeight), "Tileset Selection")
		tilesetsActive = gui.ComboBox(rl.NewRectangle(panelX, padding+labelHeight, 70 /*XXX: IDK why this cant be panel width*/, 30), tilesetNames, tilesetsActive)

		if gui.Button(rl.NewRectangle(panelX+15, 70, 30, 20), "Reload Tilesets") {
			tilesets = loadTilesets()
			tilesetNames = namesFromTilesets(tilesets)
			tileset = LoadTileset(tilesetPath + tilesetNames[tilesetsActive] + "/")
			images = LoadTilesetImages(tileset)
			mapNames = loadMapList()
			m.Tileset = tilesets[tilesetsActive]
			m.Tiledata = make([]int, m.Height*m.Width)
		}
		const lineHeight = 95
		rl.DrawLine(int32(panelLX), lineHeight, ScreenWidth, lineHeight, rl.LightGray)

		gui.Label(rl.NewRectangle(panelX, 100, panelWidth, 20), "Tile Selection")
		tileActive = gui.ComboBox(rl.NewRectangle(panelX, 120, 70 /*XXX: IDK why this cant be panel width*/, 30), tileNames, tileActive)

		if gui.Button(rl.NewRectangle(panelLX+padding*12, ScreenHeight-50, 20, 20), "+") {
			resizeMap(&m, 0, 1)
		}
		gui.Label(rl.NewRectangle(panelLX+3*padding, ScreenHeight-50, 20, 20), fmt.Sprintf("Map Rows: %d", m.Height))
		if gui.Button(rl.NewRectangle(panelLX+padding, ScreenHeight-50, 20, 20), "-") {
			resizeMap(&m, 0, -1)
		}
		if gui.Button(rl.NewRectangle(panelLX+padding*12, ScreenHeight-30, 20, 20), "+") {
			resizeMap(&m, 1, 0)
		}
		gui.Label(rl.NewRectangle(panelLX+3*padding, ScreenHeight-30, 20, 20), fmt.Sprintf("Columns: %d", m.Width))
		if gui.Button(rl.NewRectangle(panelLX+padding, ScreenHeight-30, 20, 20), "-") {
			resizeMap(&m, -1, 0)
		}
		if gui.Button(rl.NewRectangle(panelLX+padding*12, ScreenHeight-70, 20, 20), "+") {
			m.Specials++
			manageSpecials()
		}
		gui.Label(rl.NewRectangle(panelLX+3*padding, ScreenHeight-70, 20, 20), fmt.Sprintf("Specials: %d", m.Specials))
		if gui.Button(rl.NewRectangle(panelLX+padding, ScreenHeight-70, 20, 20), "-") {
			m.Specials--
			if m.Specials < 0 {
				m.Specials = 0
			}
			manageSpecials()
		}

		// End Render

		rl.DrawLine(int32(panelLX), 0, int32(panelLX), ScreenHeight, rl.Gray)

		rl.EndDrawing()
	}
	return -1
}

func loadTilesets() []Tileset {
	files, err := ioutil.ReadDir(tilesetPath)
	if err != nil {
		log.Fatal(err)
	}

	tilesets := make([]Tileset, 0)
	for _, f := range files {
		tilesets = append(tilesets, LoadTileset(tilesetPath+f.Name()))
	}
	return tilesets
}
func namesFromTilesets(ts []Tileset) []string {
	names := make([]string, len(ts))
	for i, f := range ts {
		s := strings.Split(f.Rootpath, "/")
		names[i] = s[len(s)-1]
	}
	return names
}

func loadMapList() []string {
	files, err := ioutil.ReadDir("assets/maps/")
	if err != nil {
		log.Fatal(err)
	}

	filenames := make([]string, 0)
	for _, f := range files {
		filenames = append(filenames, f.Name())
	}
	return filenames
}

func resizeMap(m *Map, dw, dh int) {
	oh := m.Height
	ow := m.Width
	m.Height += dh
	m.Width += dw
	if m.Height < 1 {
		m.Height = 1
	}
	if m.Width < 1 {
		m.Width = 1
	}
	if m.Width == ow && m.Height == oh {
		return
	} else if m.Width == ow {
		newData := make([]int, m.Width*m.Height)
		for i := 0; i < len(newData) && i < len(m.Tiledata); i++ {
			newData[i] = m.Tiledata[i]
		}
		m.Tiledata = newData
	} else {
		newData := make([]int, m.Width*m.Height)
		for y := 0; y < m.Height; y++ {
			for x := 0; x < m.Width; x++ {
				if x < ow && y < oh {
					newData[x+y*m.Width] = m.Tiledata[x+y*ow]
				} else {
					newData[x+y*m.Width] = 0
				}
			}
		}
		m.Tiledata = newData
	}

}
