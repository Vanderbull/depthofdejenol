namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data06RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA06GeneralStore : IMordorDataFile
{
    [NewRecord]
    public string Version { get; set; } = "";

    [NewRecord]
    public short Unused { get; set; }

    [NewRecord]
    public short AllItemsCount { get; set; }

    public StoreItem[] Items { get; set; } = new StoreItem[StaticValues.ItemCount];
}
