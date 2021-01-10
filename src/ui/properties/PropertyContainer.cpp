#include "PropertyContainer.h"
#include "BoolProperty.h"
#include "ColorProperty.h"
#include "DoubleProperty.h"
#include "GroupProperty.h"
#include "IntProperty.h"
#include "SelectionProperty.h"
#include "TextProperty.h"
#include "Vector3Property.h"
#include "plugin/PluginSpecification.h"

namespace PR {
namespace UI {
PropertyContainer::PropertyContainer()
{
}

PropertyContainer::~PropertyContainer()
{
}

void PropertyContainer::add(IProperty* property)
{
	if (mAllProperties.contains(property))
		return;

	mTopProperties.append(property);

	rec_add(property);
}

void PropertyContainer::rec_add(IProperty* property)
{
	if (mAllProperties.contains(property))
		return;

	mAllProperties.append(property);

	connect(property, &IProperty::propertyDestroyed, [this](IProperty* property) { this->propertyWasDestroyed(property); });
	connect(property, &IProperty::propertyChanged, [this]() { this->propertyChanged((IProperty*)this->sender()); });
	connect(property, &IProperty::propertyStructureChanged, [this]() { this->propertyStructureChanged((IProperty*)this->sender()); });
	connect(property, &IProperty::valueChanged, [this]() { this->valueChanged((IProperty*)this->sender()); });

	for (IProperty* child : property->children())
		rec_add(child);
}

void PropertyContainer::remove(IProperty* property)
{
	if (!mAllProperties.contains(property))
		return;

	mTopProperties.removeOne(property);
	mAllProperties.removeOne(property);

	disconnect(property);

	for (IProperty* child : property->children())
		remove(child);
}

const QVector<IProperty*>& PropertyContainer::allProperties() const
{
	return mAllProperties;
}

const QVector<IProperty*>& PropertyContainer::topProperties() const
{
	return mTopProperties;
}

void PropertyContainer::propertyWasDestroyed(IProperty* prop)
{
	Q_ASSERT(prop);
	remove(prop);
}

void PropertyContainer::propertyWasChanged(IProperty* obj)
{
	emit propertyChanged(obj);
}

void PropertyContainer::propertyStructureWasChanged(IProperty* obj)
{
	emit propertyStructureChanged(obj);
}

void PropertyContainer::valueWasChanged(IProperty* obj)
{
	emit valueChanged(obj);
}

void PropertyContainer::addSpecification(const PluginSpecification& spec)
{
	addBlock(spec.Inputs, nullptr);
}

void PropertyContainer::addBlock(const PluginParamDescBlock& block, IProperty* parent)
{
	// Optionals are skipped, to make use of some plugins which have different behaviour based on provided properties.
	// TODO: A better approach would be to give each property an "unset" type, to make use of this
	for (const auto& entry : block.Inputs) {
		if (std::holds_alternative<ParameterDesc>(entry)) {
			const auto desc = std::get<ParameterDesc>(entry);
			if (desc.flags() & ParameterFlag::Optional)
				continue;

			IProperty* prop = nullptr;
			if (desc.isBool()) {
				prop = new BoolProperty();
				reinterpret_cast<BoolProperty*>(prop)->setValue(std::get<ParameterBoolDesc>(desc.Value).Default);
				reinterpret_cast<BoolProperty*>(prop)->setDefaultValue(std::get<ParameterBoolDesc>(desc.Value).Default);
			} else if (desc.isInt()) {
				prop = new IntProperty();
				reinterpret_cast<IntProperty*>(prop)->setMinValue(std::get<ParameterIntDesc>(desc.Value).Min);
				reinterpret_cast<IntProperty*>(prop)->setMaxValue(std::get<ParameterIntDesc>(desc.Value).Max);
				reinterpret_cast<IntProperty*>(prop)->setValue(std::get<ParameterIntDesc>(desc.Value).Default);
				reinterpret_cast<IntProperty*>(prop)->setDefaultValue(std::get<ParameterIntDesc>(desc.Value).Default);
			} else if (desc.isUInt()) {
				prop = new IntProperty();
				reinterpret_cast<IntProperty*>(prop)->setMinValue(std::get<ParameterUIntDesc>(desc.Value).Min);
				reinterpret_cast<IntProperty*>(prop)->setMaxValue(std::get<ParameterUIntDesc>(desc.Value).Max);
				reinterpret_cast<IntProperty*>(prop)->setValue(std::get<ParameterUIntDesc>(desc.Value).Default);
				reinterpret_cast<IntProperty*>(prop)->setDefaultValue(std::get<ParameterUIntDesc>(desc.Value).Default);
			} else if (desc.isNumber()) {
				prop = new DoubleProperty();
				reinterpret_cast<DoubleProperty*>(prop)->setMinValue(std::get<ParameterNumberDesc>(desc.Value).Min);
				reinterpret_cast<DoubleProperty*>(prop)->setMaxValue(std::get<ParameterNumberDesc>(desc.Value).Max);
				reinterpret_cast<DoubleProperty*>(prop)->setValue(std::get<ParameterNumberDesc>(desc.Value).Default);
				reinterpret_cast<DoubleProperty*>(prop)->setDefaultValue(std::get<ParameterNumberDesc>(desc.Value).Default);
			} else if (desc.isScalarNode()) {
				prop = new DoubleProperty();
				reinterpret_cast<DoubleProperty*>(prop)->setMinValue(0);
				reinterpret_cast<DoubleProperty*>(prop)->setMaxValue(1000);
				reinterpret_cast<DoubleProperty*>(prop)->setValue(std::get<ParameterScalarNodeDesc>(desc.Value).Default);
				reinterpret_cast<DoubleProperty*>(prop)->setDefaultValue(std::get<ParameterScalarNodeDesc>(desc.Value).Default);
			} else if (desc.isSpectralNode()) {
				const float v	   = std::get<ParameterSpectralNodeDesc>(desc.Value).Default;
				const QColor color = QColor::fromRgbF(v, v, v);

				prop = new ColorProperty();
				reinterpret_cast<ColorProperty*>(prop)->setColor(color);
				reinterpret_cast<ColorProperty*>(prop)->setDefaultColor(color);
			} else if (desc.isVectorNode()) {
				prop = new Vector3Property();
				reinterpret_cast<Vector3Property*>(prop)->setValue(std::get<ParameterVectorNodeDesc>(desc.Value).Default);
				reinterpret_cast<Vector3Property*>(prop)->setDefaultValue(std::get<ParameterVectorNodeDesc>(desc.Value).Default);
			} else if (desc.isString()) {
				prop = new TextProperty();
				reinterpret_cast<TextProperty*>(prop)->setText(QString::fromStdString(std::get<ParameterStringDesc>(desc.Value).Default));
				reinterpret_cast<TextProperty*>(prop)->setDefaultText(QString::fromStdString(std::get<ParameterStringDesc>(desc.Value).Default));
			} else if (desc.isFilename()) {
				prop = new TextProperty(); // TODO: Special property?
				reinterpret_cast<TextProperty*>(prop)->setText(QString::fromStdString(std::get<ParameterFilenameDesc>(desc.Value).Default));
				reinterpret_cast<TextProperty*>(prop)->setDefaultText(QString::fromStdString(std::get<ParameterFilenameDesc>(desc.Value).Default));
			} else if (desc.isOptions()) {
				const auto opts = std::get<ParameterOptionDesc>(desc.Value);
				prop			= new SelectionProperty();

				for (const auto& a : opts.Options)
					reinterpret_cast<SelectionProperty*>(prop)->addItem(QString::fromStdString(a));

				auto index = std::distance(opts.Options.begin(), std::find(opts.Options.begin(), opts.Options.end(), opts.Default));
				if (index >= static_cast<decltype(index)>(opts.Options.size()))
					index = 0;

				reinterpret_cast<SelectionProperty*>(prop)->setIndex(index);
				reinterpret_cast<SelectionProperty*>(prop)->setDefaultIndex(index);
			} else if (desc.isReference()) {
				// TODO
			}

			if (prop) {
				prop->setPropertyName(QString::fromStdString(desc.normalizedName()));
				prop->setToolTip(QString::fromStdString(desc.description()));

				if (parent)
					parent->addChild(prop);
				else
					add(prop);
			}
		} else {
			const auto block = std::get<std::shared_ptr<PluginParamDescBlock>>(entry);
			switch (block->Operator) {
			default:
			case PluginParamDescBlockOp::And:
			case PluginParamDescBlockOp::Or:	// TODO
			case PluginParamDescBlockOp::OneOf: // TODO
			{
				IProperty* prop = new GroupProperty();
				prop->setPropertyName(QString::fromStdString(block->Name));

				addBlock(*block, prop);

				if (parent)
					parent->addChild(prop);
				else
					add(prop);
			} break;
			case PluginParamDescBlockOp::Optional: // Skip optionals
				continue;
			}
		}
	}
}
} // namespace UI
} // namespace PR