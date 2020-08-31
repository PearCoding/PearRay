#pragma once

#include <QAbstractItemModel>
#include <memory>

class ProfFile;
class ProfTreeItem;
class ProfTreeModel : public QAbstractItemModel {
public:
	ProfTreeModel(const std::shared_ptr<ProfFile>& ctx);
	virtual ~ProfTreeModel();

	QVariant data(const QModelIndex& index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column,
					  const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	const QVector<std::shared_ptr<ProfTreeItem>>& roots() const { return mFiles; }

private:
	void setupData();
	QVector<std::shared_ptr<ProfTreeItem>> mFiles;
	std::shared_ptr<ProfFile> mContext;
};