// TableauCardHandlerRegistry — 注册所有桌面牌类型行为
#pragma once
#include <map>
#include <memory>
#include "data/CardDef.h"
#include "ITableauCardHandler.h"

class TableauCardHandlerRegistry {
public:
    TableauCardHandlerRegistry();

    void registerHandler(CardKind kind, std::unique_ptr<ITableauCardHandler> handler);
    ITableauCardHandler* handlerFor(CardKind kind) const;

private:
    std::map<CardKind, std::unique_ptr<ITableauCardHandler>> _handlers;
};
