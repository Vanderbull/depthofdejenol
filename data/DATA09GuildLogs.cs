namespace MordorDataLibrary.Models;

[DataRecordLength(StaticValues.Data09RecordLength)]
[UsedImplicitly(ImplicitUseTargetFlags.WithMembers)]
public class DATA09GuildLogs : IMordorDataFile
{
    private short _count;

    [NewRecord]
    public short Count
    {
        get => _count;
        set
        {
            _count = value;
            if (Entries.Length == 0)
            {
                Entries = new GuildLogEntry[value];
            }
        }
    }

    public GuildLogEntry[] Entries { get; set; } = [];
}
