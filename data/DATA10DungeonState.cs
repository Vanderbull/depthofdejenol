namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data10RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA10DungeonState : IMordorDataFile
{
    [NewRecord]
    public short LevelCount { get; set; }

    [NewRecord]
    public short[] SpawnCounts { get; set; } = new short[15];

    public AreaSpawn[] AreaSpawns { get; set; } = new AreaSpawn[3000];
}
