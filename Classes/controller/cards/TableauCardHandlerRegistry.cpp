// TableauCardHandlerRegistry — 注册所有桌面牌类型行为
#include "TableauCardHandlerRegistry.h"
#include "MatchTableauHandler.h"

TableauCardHandlerRegistry::TableauCardHandlerRegistry() {
  // 如要新增功能牌，实现 ITableauCardHandler 后在此加一行 registerHandler
    registerHandler(CardKind::Normal, std::make_unique<MatchTableauHandler>());
}

void TableauCardHandlerRegistry::registerHandler(
    CardKind kind, std::unique_ptr<ITableauCardHandler> handler) {
    _handlers[kind] = std::move(handler);
}

ITableauCardHandler* TableauCardHandlerRegistry::handlerFor(CardKind kind) const {
    auto it = _handlers.find(kind);
    if (it != _handlers.end()) return it->second.get();
    auto fallback = _handlers.find(CardKind::Normal);
    return fallback != _handlers.end() ? fallback->second.get() : nullptr;
}
