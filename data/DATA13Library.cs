namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data13RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA13Library : IMordorDataFile
{
    [NewRecord]
    public string Version { get; set; } = null!;

    [NewRecord]
    public short TotalMonsters { get; set; }

    [NewRecord]
    public short TotalItems { get; set; }

    [NewRecord]
    public short MonstersFound { get; set; }

    [NewRecord]
    public short ItemsFound { get; set; }

    public LibraryRecord[] Monsters { get; set; } = new LibraryRecord[StaticValues.MonsterCount];

    public LibraryRecord[] Items { get; set; } = new LibraryRecord[StaticValues.ItemCount];
}
