namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data04RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA04Characters : IMordorDataFile
{
    [NewRecord]
    public string Version { get; set; } = null!;

    [NewRecord]
    public short PartialCharacterCount { get; set; }

    public List<Character> CharacterList { get; set; } = [];
}
