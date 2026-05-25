using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing.Drawing2D;
using System.IO;
using System.Linq;
using System.Text;
using UnityEditor;
using UnityEngine;

public class ClimberConfigGenerator : MonoBehaviour, ICodeGen
{
    public int CodeGenOrder => 99999999;
    public bool IsEnabled => this.isActiveAndEnabled;

	[HelpBox("Mobys added as children to this will automatically populate below.", MessageType.Info, placeAbove = false)]
	public float GridCellSize = 5;
	public List<ClimberMobyDef> MobyDefs = new List<ClimberMobyDef>();

#region Validate

	void ValidateChildMobys()
	{
		var childMobys = this.GetComponentsInChildren<Moby>(includeInactive: true);
		foreach (var childMoby in childMobys)
		{
			if (MobyDefs.Any(d => d.Moby == childMoby))
				continue;

			MobyDefs.Add(new ClimberMobyDef()
			{
				Name = childMoby.name,
				Moby = childMoby,
				SpawnType = ClimberSpawnType.Default,
				SpawnInHorizontal = 5,
				SpawnOutHorizontal = 5,
				SpawnInVertical = 1,
				SpawnOutVertical = 1,
			});

			childMoby.ModeBits |= 15;
		}

		MobyDefs.RemoveAll(x => x == null || !x.Moby || !childMobys.Contains(x.Moby));
	}

	void OnValidate()
	{
		ValidateChildMobys();

		var gridSize = Mathf.CeilToInt(Mathf.Sqrt(MobyDefs.Count));
		var idx = 0;
		foreach (var mobyDef in MobyDefs)
		{
			if (mobyDef.Moby)
			{
				mobyDef.Name = mobyDef.Moby.name;
				
				// var asset = UnityHelper.GetAssetPrefab(FolderNames.MobyFolder, mobyDef.Moby.OClass.ToString(), mobyDef.Moby.RCVersion);
				// var assetPath = AssetDatabase.GetAssetPath(asset);
				// var binPath = Path.Combine(Path.GetDirectoryName(assetPath), "core.bin");
				// if (File.Exists(binPath))
				// {
				// 	var binData = File.ReadAllBytes(binPath);
				// 	mobyDef.BaseMobyScale = 0.11f / BitConverter.ToSingle(binData, 0x24);
				// }

				var p = Vector3.right * GridCellSize * (idx % gridSize);
				p += Vector3.forward * GridCellSize * (idx / gridSize);
				mobyDef.Moby.transform.localPosition = p;
			}
			else
			{
				mobyDef.Name = "undefined";
			}

			++idx;
		}
	}

	#endregion

	#region Gizmos

	void DrawClimberGizmos(ClimberMobyDef mobyDef)
	{
		const float minRandScaleH = 1f;
		const float maxRandScaleH = 2f;
		const float minRandScaleV = 1.5f;
		const float maxRandScaleV = 3f;

		var m = Gizmos.matrix;

		var minScale = new Vector3(
			  minRandScaleH * mobyDef.SpawnOutHorizontal
			, minRandScaleV * mobyDef.SpawnOutVertical
			, minRandScaleH * mobyDef.SpawnOutHorizontal);

		var maxScale = new Vector3(
			  maxRandScaleH * mobyDef.SpawnOutHorizontal
			, maxRandScaleV * mobyDef.SpawnOutVertical
			, maxRandScaleH * mobyDef.SpawnOutHorizontal);

		Gizmos.color = new Color(1, 0, 0, 0.25f);
		Gizmos.matrix = Matrix4x4.TRS(mobyDef.Moby.transform.position, Quaternion.identity, minScale);
		Gizmos.DrawSphere(Vector3.zero, 1f);

		Gizmos.color = new Color(0, 1, 0, 0.25f);
		Gizmos.matrix = Matrix4x4.TRS(mobyDef.Moby.transform.position, Quaternion.identity, maxScale);
		Gizmos.DrawSphere(Vector3.zero, 1f);

		Gizmos.matrix = m;
	}

	void OnDrawGizmosSelected()
	{
		foreach (var mobyDef in MobyDefs)
			DrawClimberGizmos(mobyDef);
	}

	void OnDrawGizmos()
	{
		foreach (var mobyDef in MobyDefs)
		{
			if (Selection.activeGameObject == mobyDef.Moby.gameObject)
			{
				DrawClimberGizmos(mobyDef);
			}
		}
	}

	#endregion

	#region Configure

	public void Configure(string buildFolder, CodeGenState state)
	{
		var srcFolder = Path.Combine(buildFolder, FolderNames.CodeBuildSrcFolder);
		
		state.ObjectFiles.Add($"{FolderNames.CodeBuildSrcFolder}/config.o");
		File.WriteAllText(Path.Combine(srcFolder, "config.c"), BuildConfig());
	}

	string BuildConfig()
	{
		var sb = new StringBuilder();

		sb.AppendLine("#include <libdl/utils.h>");
		sb.AppendLine("#include \"config.h\"");
		sb.AppendLine();

		sb.AppendLine("ClimberMobyDef_t climberMobyDefs[] = {");
		foreach (var def in MobyDefs)
		{
			if (!def.Moby || !def.Moby.isActiveAndEnabled)
				continue;

			sb.AppendLine($"\t{{ // {def.Name}");
			sb.AppendLine($"\t\t.InstanceUid = {def.Moby.Uid},");
			sb.AppendLine($"\t\t.OClass = {def.Moby.OClass},");
			sb.AppendLine($"\t\t.PVarSize = {def.Moby.GetPVarData()?.Length ?? 0},");
			sb.AppendLine($"\t\t.SpawnType = {def.SpawnType.GetDescription()},");
			sb.AppendLine($"\t\t.ObjectScale = {def.Moby.transform.localScale.x},");
			sb.AppendLine($"\t\t.SpawnInHorizontal = {def.SpawnInHorizontal},");
			sb.AppendLine($"\t\t.SpawnOutHorizontal = {def.SpawnOutHorizontal},");
			sb.AppendLine($"\t\t.SpawnInVertical = {def.SpawnInVertical},");
			sb.AppendLine($"\t\t.SpawnOutVertical = {def.SpawnOutVertical},");
			sb.AppendLine("\t},");
		}
		sb.AppendLine("};");
		sb.AppendLine("int climberMobyDefsCount = COUNT_OF(climberMobyDefs);");

		return sb.ToString();
	}

#endregion

	[Serializable]
	public enum ClimberSpawnType
	{
		[Description("CLIMBER_SPAWN_DEFAULT")]
		Default,
    [Description("CLIMBER_SPAWN_PLATFORM")]
    Platform,
    [Description("CLIMBER_SPAWN_WITH_PUPDATE")]
    WithPUpdate,
	}

	[Serializable]
	public class ClimberMobyDef
	{
		[HideInInspector]
		public string Name;
		[ReadOnly]
		public Moby Moby;
		public ClimberSpawnType SpawnType;
		public float SpawnInHorizontal;
		public float SpawnOutHorizontal;
		public float SpawnInVertical;
		public float SpawnOutVertical;
	}
}
