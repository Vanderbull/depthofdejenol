#ifndef DUNGEONMAP_H
#define DUNGEONMAP_H

#include <QWidget>
#include <vector>

class DungeonMap : public QWidget {
    Q_OBJECT
public:
    explicit DungeonMap(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<std::vector<int>> mapData;
    void generateMap();
};

#endif // DUNGEONMAP_H
