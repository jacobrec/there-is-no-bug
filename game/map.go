package game

import (
	rl "github.com/gen2brain/raylib-go/raylib"

	"bytes"
	"encoding/gob"
	"fmt"
	"io/ioutil"
	"log"
)

type Tileset struct {
	Rootpath string
	Tiles    []string
}

type Map struct {
	Filepath string
	Width    int
	Height   int
	Specials int
	Tileset  Tileset
	Tiledata []int
}

func LoadTileset(path string) Tileset {
	files, err := ioutil.ReadDir(path)
	if err != nil {
		log.Fatal(err)
	}

	tilenames := make([]string, 0)
	for _, f := range files {
		tilenames = append(tilenames, f.Name())
	}
	return Tileset{
		Rootpath: path,
		Tiles:    tilenames,
	}
}

func LoadTilesetImages(tileset Tileset) []rl.Texture2D {
	images := make([]rl.Texture2D, len(tileset.Tiles))
	for i := 0; i < len(images); i++ {
		images[i] = rl.LoadTexture(tileset.Rootpath + tileset.Tiles[i])
	}
	return images
}

func LoadMap(filename string) Map {
	mapFile := "assets/maps/" + filename
	data, err := ioutil.ReadFile(mapFile)
	dec := gob.NewDecoder(bytes.NewReader(data))
	var m Map
	if err == nil {
		err = dec.Decode(&m)
	} else {
		fmt.Println("Could not find mapfile: " + mapFile)
	}
	if err != nil {
		fmt.Println("Could not parse mapfile")
		return Map{
			Filepath: mapFile,
			Height:   10,
			Width:    10,
			Specials: 0,
			Tiledata: make([]int, 100),
			Tileset:  LoadTileset("assets/art/tilesets/bonky_kong/"),
		}
	}
	return m
}
func SaveMap(m *Map) bool {
	var buf bytes.Buffer
	enc := gob.NewEncoder(&buf)
	err := enc.Encode(m)
	if err == nil {
		err = ioutil.WriteFile(m.Filepath, buf.Bytes(), 0644)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Println("Saving file: " + m.Filepath)
	}
	return err == nil
}
