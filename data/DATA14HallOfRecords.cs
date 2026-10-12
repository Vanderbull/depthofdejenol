namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data14RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA14HallOfRecords : IMordorDataFile
{
    public HallRecord[] Records { get; set; } = new HallRecord[13];
}
