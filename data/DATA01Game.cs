namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data01RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA01GameData : IMordorDataFile
{
    [NewRecord]
    public string Version { get; set; } = null!;

    [NewRecord]
    public short NumRaces { get; set; }

    [NewRecord]
    public short NumGuilds { get; set; }

    [NewRecord]
    public short NumItemSubtypes { get; set; }

    [NewRecord]
    public short NumItemTypes { get; set; }

    [NewRecord]
    public short NumMonsterSubtypes { get; set; }

    [NewRecord]
    public short NumMonsterTypes { get; set; }

    public Race[] Races { get; set; } = new Race[9];

    public Guild[] Guilds { get; set; } = new Guild[12];

    public ItemSubtype[] ItemSubtypes { get; set; } = new ItemSubtype[35];

    public ItemType[] ItemTypes { get; set; } = new ItemType[24];

    public MonsterSubtype[] MonsterSubtypes { get; set; } = new MonsterSubtype[18];

    public MonsterType[] MonsterTypes { get; set; } = new MonsterType[15];
}
