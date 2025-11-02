namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data02RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA02Spells : IMordorDataFile
{
    [NewRecord]
    public string Version { get; set; } = null!;

    [NewRecord]
    public short Count { get; set; }

    public Spell[] Spells { get; set; } = new Spell[105];
}
