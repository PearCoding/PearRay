#pragma once

#include "IProperty.h"

class QLineEdit;
class TextProperty : public IProperty
{
	Q_OBJECT
public:
	TextProperty();
	virtual ~TextProperty();

	QString valueText() const;
	void undo();
	void save();
	QWidget* editorWidget(QWidget* parent);

	void setText(const QString& str);
	QString text() const;

	void setDefaultText(const QString& str);
	QString defaultText() const;

private slots:
	void lineChanged(const QString& str);

private: 
	QLineEdit* mWidget;
	QString mDefaultText;
	QString mText;
};
