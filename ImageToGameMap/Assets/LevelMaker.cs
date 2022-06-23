using UnityEngine;
using SFB;
using System.IO;
using System.Collections.Generic;

public class LevelMaker : MonoBehaviour{

	string[] hText;
	string[] cText;

	[System.Serializable]
	class JSONArray{
		public ColorToText[] level;
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
			}
			paths = dynamicPaths.ToArray();

			cText = new string[paths.Length];
			hText = new string[paths.Length];
			for (int i = 0; i < paths.Length; i++){
				var rawData = File.ReadAllBytes(paths[i]);
				Texture2D map = new Texture2D(2, 2);
				map.LoadImage(rawData);

				string textImage = "const u8 level_" + (i + 1) + "[" + map.height + "][" + map.width + "] = {\r\n";

				for (int y = map.height - 1, y0 = 0; y >= 0; y--, y0++){
					textImage += "\t{ ";
					for (int x = 0; x < map.width; x++){
						Color pixelColor = map.GetPixel(x, y);

						if (pixelColor.a == 0){
							textImage += '0' + ", ";
						}else{
							textImage += PaintTile(pixelColor) + ", ";
						}
					}
					if (y != 0)
						textImage += "},\r\n";
				}

				textImage += "},\r\n};";

				cText[i] += textImage;
				hText[i] += "extern const u8 level_" + (i + 1) + "[" + map.height + "][" + map.width + "];";
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
