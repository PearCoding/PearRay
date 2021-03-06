#pragma once

#include <QPixmap>
#include <QVariant>
#include <QVector>

#include <memory>

class ProfFile;

class ProfTreeItem : public std::enable_shared_from_this<ProfTreeItem> {
public:
	enum Columns {
		C_Name = 0,
		C_TotalValue,
		C_TotalDuration,
		C_AverageDuration,
		_C_COUNT
	};

	ProfTreeItem(const std::shared_ptr<ProfTreeItem>& parent, QString name, ProfFile* file, int index = -1);
	virtual ~ProfTreeItem();

	quint64 totalValue() const;
	quint64 totalDuration() const;

	quint64 totalValue(quint64 t) const;
	quint64 totalDuration(quint64 t) const;

	QVector<quint64> timePoints() const;

	inline void setName(const QString& name) { mName = name; }
	inline const QString& name() const { return mName; }

	inline void setParent(const std::shared_ptr<ProfTreeItem>& parent) { mParent = parent; }
	inline std::shared_ptr<ProfTreeItem> parent() const { return mParent; }

	void addChild(const std::shared_ptr<ProfTreeItem>& item);
	void removeChild(const std::shared_ptr<ProfTreeItem>& item);
	std::shared_ptr<ProfTreeItem> child(int row) const;
	inline const QVector<std::shared_ptr<ProfTreeItem>>& children() const { return mChildren; }
	inline bool isLeaf() const { return mIndex >= 0; }

	QVariant data(int column) const;
	int row() const;

	inline void setCustomIcon(const QPixmap& p) { mCustomIcon = p; }
	inline const QPixmap& customIcon() const { return mCustomIcon; }

private:
	QString mName;
	std::shared_ptr<ProfTreeItem> mParent;
	QVector<std::shared_ptr<ProfTreeItem>> mChildren;

	mutable QVector<quint64> mTimePoints_Cached;

	ProfFile* mFile;
	int mIndex;

	QPixmap mCustomIcon;
};
