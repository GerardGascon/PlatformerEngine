using UnityEngine;
using SFB;
using System.IO;
using System.Collections.Generic;
//using EntityTypes;

public static class Extensions{
	public static bool IsEqual(this EntityType type, params EntityType[] types){
		for (int i = 0; i < types.Length; i++){
			if (type == types[i])
				return true;
		}

		return false;
	}
}

[System.Serializable] public enum EntityType { Box, BigBox, Coin1, Coin2, Coin5, Coin10, Coin20, Coin50, WalkerEnemy, PacerEnemy, FloaterEnemy, FallerEnemy, FallingPlatform, FloaterFollowerEnemy };

public class LevelMaker : MonoBehaviour{

	string[] hText;
	string[] cText;

	[System.Serializable]
	class JSONArray{
		public ColorToText[] level;
		public Entity[] entities;
	}

	[System.Serializable]
	class ColorToText{
		public ColorToText(string color, short number){
			this.color = color;
			this.number = number;
		}

		public string color;
		public short number;
	}

	[System.Serializable]
	class Entity{
		public EntityType type;
		public string color;
	}
	[System.Serializable]
	class StringEntity{
		public EntityType type;
		public string str;

		public StringEntity(Vector2Int position, EntityType type){
			this.type = type;
			switch (type){
				case EntityType.Box:
				case EntityType.BigBox:
					str = "{{" + position.x + ", " + position.y + "}, " + (type == EntityType.BigBox ? 1 : 0) + "},";
					break;
				case EntityType.Coin1:
				case EntityType.Coin2:
				case EntityType.Coin5:
				case EntityType.Coin10:
				case EntityType.Coin20:
				case EntityType.Coin50:
					str = "{{" + position.x + ", " + position.y + "}, " + ((int)type - 2) + "},";
					break;
				case EntityType.WalkerEnemy:
				case EntityType.PacerEnemy:
				case EntityType.FloaterEnemy:
				case EntityType.FallingPlatform:
				case EntityType.FloaterFollowerEnemy:
				case EntityType.FallerEnemy:
					str = "{{" + position.x + ", " + position.y + "}},";
					break;
			}
		}
	}

	JSONArray jsonArray;

	public void SelectJSON(){
		var extensions = new[] {
			new ExtensionFilter("JSON", "json"),
		};
		var paths = StandaloneFileBrowser.OpenFilePanel("Open File", PlayerPrefs.GetString("CSVFolder", Application.dataPath), extensions, false);

		if (paths.Length != 0){
			PlayerPrefs.SetString("CSVFolder", Path.GetDirectoryName(paths[0]));
			var reader = new StreamReader(paths[0]);
			jsonArray = JsonUtility.FromJson<JSONArray>(reader.ReadToEnd());
		}
	}

	public void SelectFile(){
		if (jsonArray == null)
			return;

		var extensions = new[] {
			new ExtensionFilter("PNG", "png"),
			new ExtensionFilter("JPG", "jpg", "jpeg", "jpe", "jfif"),
			new ExtensionFilter("Bitmap", "bmp"),
			new ExtensionFilter("TIFF", "tiff", "tif"),
		};
		var paths = StandaloneFileBrowser.OpenFilePanel("Open File", PlayerPrefs.GetString("ImageFolder", Application.dataPath), extensions, true);

		List<string> dynamicPaths = new List<string>(paths);
		if(paths.Length != 0){
			PlayerPrefs.SetString("ImageFolder", Path.GetDirectoryName(paths[0]));
			for (int i = 0; i < paths.Length; i++){
				var rawData = File.ReadAllBytes(paths[i]);
				Texture2D map = new Texture2D(2, 2);
				map.LoadImage(rawData);

				if(map.height != 14){
					dynamicPaths.Remove(paths[i]);
				}
			}
			paths = dynamicPaths.ToArray();

			cText = new string[paths.Length];
			hText = new string[paths.Length];
			for (int i = 0; i < paths.Length; i++){
				var rawData = File.ReadAllBytes(paths[i]);
				Texture2D map = new Texture2D(2, 2);
				map.LoadImage(rawData);

				List<StringEntity> entities = new List<StringEntity>();

				string textImage = "const u8 screen1_" + (i + 1) + "[" + map.height + "][" + map.width + "] = {\r\n";

				for (int y = map.height - 1, y0 = 0; y >= 0; y--, y0++){
					textImage += "\t{ ";
					for (int x = 0; x < map.width; x++){
						Color pixelColor = map.GetPixel(x, y);

						if (pixelColor.a == 0){
							textImage += '0' + ", ";
						}else{
							textImage += PaintTile(pixelColor) + ", ";

							if(GetEntity(pixelColor) != null)
								entities.Add(new StringEntity(new Vector2Int(x, y0), GetEntity(pixelColor).type));
						}
					}
					if (y != 0)
						textImage += "},\r\n";
				}

				textImage += "},\r\n};";

				string[] entitiesText = GenerateScreenEntities(entities.ToArray(), i + 1);
				cText[i] = entitiesText[0];
				hText[i] = entitiesText[1];

				cText[i] += textImage;
				hText[i] += "extern const u8 screen1_" + (i + 1) + "[" + map.height + "][" + map.width + "];";
			}
		}
	}

	int PaintTile(Color color){
		foreach(ColorToText col in jsonArray.level){
			ColorUtility.TryParseHtmlString(col.color, out Color c);
			if (c.Equals(color)){
				return col.number;
			}
		}
		return 0;
	}

	Entity GetEntity(Color color){
		foreach(Entity e in jsonArray.entities){
			ColorUtility.TryParseHtmlString(e.color, out Color col);
			if (col == color){
				return e;
			}
		}
		return null;
	}

	string[] GenerateScreenEntities(StringEntity[] entities, int screen){
		string[] output = new string[2];
		string[] boxes = new string[2];
		string[] coins = new string[2];
		string[] walkerEnemies = new string[2];
		string[] floaterEnemies = new string[2];
		string[] pacerEnemies = new string[2];
		string[] floaterFollowerEnemies = new string[2];
		string[] fallerEnemies = new string[2];
		string[] fallingPlatforms = new string[2];

		List<StringEntity> boxEntities = new List<StringEntity>();
		List<StringEntity> coinEntities = new List<StringEntity>();
		List<StringEntity> walkerEntities = new List<StringEntity>();
		List<StringEntity> floaterEntities = new List<StringEntity>();
		List<StringEntity> pacerEntities = new List<StringEntity>();
		List<StringEntity> floaterFollowerEntities = new List<StringEntity>();
		List<StringEntity> fallerEntities = new List<StringEntity>();
		List<StringEntity> fallingPlatformEntities = new List<StringEntity>();
		for (int i = 0; i < entities.Length; i++){
			if (entities[i].type.IsEqual(EntityType.Box, EntityType.BigBox)){
				boxEntities.Add(entities[i]);
			}else if (entities[i].type.IsEqual(EntityType.Coin1, EntityType.Coin2, EntityType.Coin5, EntityType.Coin10, EntityType.Coin20, EntityType.Coin50)){
				coinEntities.Add(entities[i]);
			}else if (entities[i].type.IsEqual(EntityType.WalkerEnemy)){
				walkerEntities.Add(entities[i]);
			}else if (entities[i].type.IsEqual(EntityType.PacerEnemy)){
				pacerEntities.Add(entities[i]);
			}else if (entities[i].type.IsEqual(EntityType.FloaterFollowerEnemy)) {
				floaterFollowerEntities.Add(entities[i]);
			}else if (entities[i].type.IsEqual(EntityType.FallingPlatform)){
				fallingPlatformEntities.Add(entities[i]);
			}else if(entities[i].type.IsEqual(EntityType.FallerEnemy)){
				fallerEntities.Add(entities[i]);
			}else if(entities[i].type.IsEqual(EntityType.FloaterEnemy)){
				floaterEntities.Add(entities[i]);
			}
		}

		//Boxes
		if(boxEntities.Count != 0){
			string cBoxes = $"const BoxEntity screen1_{screen}Boxes[{boxEntities.Count}] = " + "{\r\n";
			string hBoxes = $"extern const BoxEntity screen1_{screen}Boxes[{boxEntities.Count}];\r\n";
			for (int i = 0; i < boxEntities.Count; i++){
				cBoxes += "\t" + boxEntities[i].str + "\r\n";
			}
			cBoxes += "};\r\n";
			boxes[0] = cBoxes;
			boxes[1] = hBoxes;
		}

		//Coins
		if(coinEntities.Count != 0){			
			string cCoins = $"const CurrencyEntity screen1_{screen}Currency[{coinEntities.Count}] = " + "{\r\n";
			string hCoins = $"extern const CurrencyEntity screen1_{screen}Currency[{coinEntities.Count}];\r\n";
			for (int i = 0; i < coinEntities.Count; i++){
				cCoins += "\t" + coinEntities[i].str + "\r\n";
			}
			cCoins += "};\r\n";
			coins[0] = cCoins;
			coins[1] = hCoins;
		}

		//WalkerEnemies
		if (walkerEntities.Count != 0){
			string cWalkerEnemies = $"const Entity screen1_{screen}WalkerEnemies[{walkerEntities.Count}] = " + "{\r\n";
			string hWalkerEnemies = $"extern const Entity screen1_{screen}WalkerEnemies[{walkerEntities.Count}];\r\n";
			for (int i = 0; i < walkerEntities.Count; i++){
				cWalkerEnemies += "\t" + walkerEntities[i].str + "\r\n";
			}
			cWalkerEnemies += "};\r\n";
			walkerEnemies[0] = cWalkerEnemies;
			walkerEnemies[1] = hWalkerEnemies;
		}

		//FloaterEnemies
		if (floaterEntities.Count != 0) {
			string cFloaterEnemies = $"const Entity screen1_{screen}FloaterEnemies[{floaterEntities.Count}] = " + "{\r\n";
			string hFloaterEnemies = $"extern const Entity screen1_{screen}FloaterEnemies[{floaterEntities.Count}];\r\n";
			for (int i = 0; i < floaterEntities.Count; i++) {
				cFloaterEnemies += "\t" + floaterEntities[i].str + "\r\n";
			}
			cFloaterEnemies += "};\r\n";
			floaterEnemies[0] = cFloaterEnemies;
			floaterEnemies[1] = hFloaterEnemies;
		}

		//PacerEnemies
		if (pacerEntities.Count != 0){
			string cPacerEnemies = $"const Entity screen1_{screen}PacerEnemies[{pacerEntities.Count}] = " + "{\r\n";
			string hPacerEnemies = $"extern const Entity screen1_{screen}PacerEnemies[{pacerEntities.Count}];\r\n";
			for (int i = 0; i < pacerEntities.Count; i++){
				cPacerEnemies += "\t" + pacerEntities[i].str + "\r\n";
			}
			cPacerEnemies += "};\r\n";
			pacerEnemies[0] = cPacerEnemies;
			pacerEnemies[1] = hPacerEnemies;
		}

		//FloaterFollowerEnemies
		if (floaterFollowerEntities.Count != 0) {
			string cFloaterFollowerEnemies = $"const Entity screen1_{screen}FloaterFollowerEnemies[{floaterFollowerEntities.Count}] = " + "{\r\n";
			string hFloaterFollowerEnemies = $"extern const Entity screen1_{screen}FloaterFollowerEnemies[{floaterFollowerEntities.Count}];\r\n";
			for (int i = 0; i < floaterFollowerEntities.Count; i++) {
				cFloaterFollowerEnemies += "\t" + floaterFollowerEntities[i].str + "\r\n";
			}
			cFloaterFollowerEnemies += "};\r\n";
			floaterFollowerEnemies[0] = cFloaterFollowerEnemies;
			floaterFollowerEnemies[1] = hFloaterFollowerEnemies;
		}

		//FallerEnemies
		if (fallerEntities.Count != 0) {
			string cFallerEnemies = $"const Entity screen1_{screen}FallerEnemies[{fallerEntities.Count}] = " + "{\r\n";
			string hFallerEnemies = $"extern const Entity screen1_{screen}FallerEnemies[{fallerEntities.Count}];\r\n";
			for (int i = 0; i < fallerEntities.Count; i++) {
				cFallerEnemies += "\t" + fallerEntities[i].str + "\r\n";
			}
			cFallerEnemies += "};\r\n";
			fallerEnemies[0] = cFallerEnemies;
			fallerEnemies[1] = hFallerEnemies;
		}

		//FallingPlatforms
		if (fallingPlatformEntities.Count != 0){
			string cFallingPlatforms = $"const Entity screen1_{screen}FallingPlatforms[{fallingPlatformEntities.Count}] = " + "{\r\n";
			string hFallingPlatforms = $"extern const Entity screen1_{screen}FallingPlatforms[{fallingPlatformEntities.Count}];\r\n";
			for (int i = 0; i < fallingPlatformEntities.Count; i++){
				cFallingPlatforms += "\t" + fallingPlatformEntities[i].str + "\r\n";
			}
			cFallingPlatforms += "};\r\n";
			fallingPlatforms[0] = cFallingPlatforms;
			fallingPlatforms[1] = hFallingPlatforms;
		}

		output[0] += coins[0] + boxes[0] + walkerEnemies[0] + pacerEnemies[0] + fallingPlatforms[0] + floaterFollowerEnemies[0] + fallerEnemies[0] + floaterEnemies[0];
		output[1] += coins[1] + boxes[1] + walkerEnemies[1] + pacerEnemies[1] + fallingPlatforms[1] + floaterFollowerEnemies[1] + fallerEnemies[1] + floaterEnemies[1];

		return output;
	}

	public void SaveFile(){
		if (string.IsNullOrEmpty(cText[0]) || jsonArray == null)
			return;

		var path = StandaloneFileBrowser.OpenFolderPanel("", PlayerPrefs.GetString("SaveFolder"), false);
		if (path.Length > 0){
			string dirName = path[0];
			PlayerPrefs.SetString("SaveFolder", dirName);
			StreamWriter hStream = new StreamWriter(dirName + @"\map.h", false);
			hStream.Write("#pragma once");
			hStream.Write("\r\n\r\n");
			for (int i = 0; i < hText.Length; i++){
				hStream.Write(hText[i]);
				hStream.Write("\r\n");
			}
			hStream.Close();

			StreamWriter cStream = new StreamWriter(dirName + @"\map.c", false);
			for (int i = 0; i < cText.Length; i++){
				cStream.Write(cText[i]);
				cStream.Write("\r\n");
			}
			cStream.Close();
		}
	}
}
