#include "GuildFactory.h"

QList<QVariantMap> GuildFactory::createGuildMasters() {
    QList<QVariantMap> leaders; // Changed type here

    auto addLeader = [&](QString ach, QString name, QString date, QVariant val, QString unit) {
        QVariantMap record;
        record["Achievement"] = ach;
        record["Name"] = name;
        record["Date"] = date;
        record["RecordValue"] = val;
        record["RecordUnit"] = unit;
        leaders.append(record);
    };

    addLeader("Strongest", "Goch", "4/15/2001", 25, "Strength");
    // ... rest of your data ...
    addLeader("Master of Thieving", "Healer", "5/17/2002", 22, "Thieving Skill");

    return leaders;
}
