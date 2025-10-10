#include <QWidget>
#include <QKeyEvent>
#include <QDebug> 

class GameController : public QWidget
{
    Q_OBJECT

public:
    explicit GameController(QWidget *parent = nullptr);

protected:
    // Reimplement the key press event handler
    void keyPressEvent(QKeyEvent *event) override;

private:
    // Your custom function to be called on arrow key press
    void handleArrowPress(Qt::Key key);
};
