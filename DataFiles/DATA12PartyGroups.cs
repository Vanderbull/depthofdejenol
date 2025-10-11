namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data12RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA12PartyGroups : IMordorDataFile
{
    [NewRecord]
    public short FakeCount { get; set; }

    public List<Party> Parties { get; set; } = [];
}
