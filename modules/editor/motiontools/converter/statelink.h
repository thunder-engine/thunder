#ifndef STATELINK_H
#define STATELINK_H

#include <editor/graph/abstractnodegraph.h>
#include <resources/animationstatemachine.h>

#include "animationcontrollergraph.h"
#include "entrystate.h"

#include <pugixml.hpp>

namespace {
    const char *gCondition("condition");
    const char *gDuration("duration");

    const char *gName("name");
    const char *gType("type");
    const char *gValue("value");
};

class StateLink : public GraphLink {
    A_OBJECT(StateLink, GraphLink, Graph)

    A_PROPERTIES(
        A_PROPERTY(float, duration, StateLink::duration, StateLink::setDuration)
    )

public:
    float duration() const {
        return m_duration;
    }

    void setDuration(float duration) {
        m_duration = duration;
    }

private:
    void toXml(pugi::xml_node &element) override {
        element.append_attribute(gDuration) = m_duration;

        AnimationControllerGraph *graph = static_cast<AnimationControllerGraph *>(sender->graph());
        if(graph) {
            for(auto &it : property(gCondition).toList()) {
                VariantMap data(it.toMap());
                if(data.size() >= 3) {
                    pugi::xml_node condition = element.append_child(gCondition);
                    TString name(data[gName].toString());
                    if(!name.isEmpty()) {
                        condition.append_attribute(gName) = name.data();
                        condition.append_attribute(gType) = data[gType].toInt();
                        condition.append_attribute(gValue) = data[gValue].toString().data();
                    }
                }
            }
        }
    }

    void fromXml(const pugi::xml_node &element) override {
        m_duration = element.attribute(gDuration).as_float();

        VariantList conditions;

        pugi::xml_node sub = element.first_child();
        while(sub) {
            TString conditionName(sub.attribute(gName).as_string());

            VariantMap condition;

            condition[gName] = conditionName;
            condition[gType] = sub.attribute(gType).as_int();

            AnimationControllerGraph *graph = static_cast<AnimationControllerGraph *>(sender->graph());
            if(graph) {
                pugi::xml_attribute v = sub.attribute(gValue);
                Variant variable(graph->variable(conditionName));
                switch (variable.type()) {
                    case MetaType::BOOLEAN: condition[gValue] = v.as_bool(); break;
                    case MetaType::INTEGER: condition[gValue] = v.as_int(); break;
                    case MetaType::FLOAT: condition[gValue] = v.as_float(); break;
                    default: break;
                }
            }
            conditions.push_back(condition);

            sub = sub.next_sibling();
        }

        setProperty(gCondition, conditions);
    }

    void setEndpoints(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) override {
        GraphLink::setEndpoints(sender, oport, receiver, iport);

        EntryState *entry = dynamic_cast<EntryState *>(sender);
        if(entry == nullptr) {
            setProperty(gCondition, VariantList());
            setDynamicPropertyInfo(gCondition, "editor=Condition");
        }
    }

private:
    float m_duration = 0.0f;

};

#endif // STATELINK_H
