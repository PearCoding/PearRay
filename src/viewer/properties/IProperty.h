#pragma once

#include <QString>
#include <QList>
#include <QObject>

class IProperty : public QObject
{
	Q_OBJECT
public:
	IProperty();
	virtual ~IProperty();

	QString toolTip() const;
	QString statusTip() const;
	QString whatsThis() const;
	QString propertyName() const;
	bool isReadOnly() const;
	bool isEnabled() const;
	bool isModified() const;
	bool isHeader() const;
	bool hasNoName() const;

	void setToolTip(const QString& str);
	void setStatusTip(const QString& str);
	void setWhatsThis(const QString& str);
	void setPropertyName(const QString& str);
	void setReadOnly(bool b);
	void setEnabled(bool b);
	void setModified(bool b);

	void makeHeader(bool b);
	void makeNoName(bool b);

	void addChild(IProperty* property);
	void removeChild(IProperty* property);
	QList<IProperty*> childs() const;

	virtual QString valueText() const = 0;

	// Back to the unmodified value
	virtual void undo() = 0;

	// Make the modified value as unmodified
	// Same as setModified(false);
	virtual void save() = 0;

	virtual QWidget* editorWidget(QWidget* parent) = 0;

signals:
	void propertyDestroyed(IProperty* prop);

	// Emitted when property descriptions are changed
	void propertyChanged();
	// Emitted when property descriptions and child relations are changed
	void propertyStructureChanged();

	// Emitted when property value is changed
	void valueChanged();

private: 
	QString mToolTip;
	QString mStatusTip;
	QString mWhatsThis;
	QString mPropertyName;
	bool mIsReadOnly;
	bool mIsEnabled;
	bool mIsModified;
	bool mIsHeader;
	bool mHasNoName;

	QList<IProperty*> mChilds;
};
