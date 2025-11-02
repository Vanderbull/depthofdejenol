namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data08RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA08Automap : IMordorDataFile
{
    [NewRecord]
    public string Version { get; set; } = null!;

    [NewRecord]
    public short DeepestLevel { get; set; }

    public AutomapFloor[] Floors { get; set; } = new AutomapFloor[15];
}
