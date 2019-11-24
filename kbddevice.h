#ifndef KBDDEVICE_H
#define KBDDEVICE_H

#include <QObject>
#include <QWidget>
#include <QSemaphore>

class KbdDevice : public QWidget
{
	Q_OBJECT
public:

	enum	KBD_DEVICE_KEYS {
			KBD_DEVICE_NO_KEY	= -1
	};

	explicit KbdDevice(QWidget *parent = nullptr);
	~KbdDevice();
	int ReadKey(void);								// non-blocking keboard read
	int GetKey(void);								// blocking keyboard read

signals:

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

public slots:

private:
	void HandleWaitTimer(void);

	bool		keyPressed;
	int 		currentKey;
	int 		keyCount;
	QSemaphore	access;
};

#endif // KBDDEVICE_H
