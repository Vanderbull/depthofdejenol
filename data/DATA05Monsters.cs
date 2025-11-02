namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data05RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA05Monsters : IMordorDataFile
{
    [NewRecord]
    public string Version { get; set; } = null!;

    [NewRecord]
    public short Unused { get; set; }

    [NewRecord]
    public short Count { get; set; }

    public Monster[] MonstersList { get; set; } = new Monster[StaticValues.MonsterCount];
}
