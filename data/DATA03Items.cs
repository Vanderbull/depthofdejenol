namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data03RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA03Items : IMordorDataFile
{
    [NewRecord]
    public string Version { get; set; } = "";

    [NewRecord]
    public short GeneralStoreCode { get; set; }

    [NewRecord]
    public short Count { get; set; }

    public Item[] ItemList { get; set; } = new Item[StaticValues.ItemCount];
}
