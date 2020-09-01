#pragma once

#include <QList>
#include <QSignalMapper>
#include <QString>

#include "IProperty.h"

namespace PR {
namespace UI {
class PR_LIB_UI PropertyContainer : public QObject {
	Q_OBJECT
public:
	PropertyContainer();
	~PropertyContainer();

	void add(IProperty* property);
	void remove(IProperty* property);

	const QVector<IProperty*>& topProperties() const;
	const QVector<IProperty*>& allProperties() const;

signals:
	void propertyChanged(IProperty* prop);
	void propertyStructureChanged(IProperty* prop);
	void valueChanged(IProperty* prop);

private slots:
	void propertyWasDestroyed(IProperty* prop);
	void propertyWasChanged(IProperty* obj);
	void propertyStructureWasChanged(IProperty* obj);
	void valueWasChanged(IProperty* obj);

private:
	void rec_add(IProperty* property);

	QVector<IProperty*> mAllProperties;
	QVector<IProperty*> mTopProperties;
};
} // namespace UI
} // namespace PR