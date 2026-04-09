import os
import json

tileMapDir = 'Level_Editor/TileMap/'

tileMapFiles = os.listdir(tileMapDir)

for tileMapFile in tileMapFiles:
    filePath = tileMapDir + tileMapFile
    with open(filePath, '+r') as file:
        data = json.load(file)
        if 'tilesets' in data:
            for tileset in data['tilesets']:
                tileset['source'] = '../TileSet/game_tilesets.tsj'
                print(tileset['source'])
        
        file.seek(0)
        json.dump(data, file)