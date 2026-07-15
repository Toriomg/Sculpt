// Abstract editor command; subclasses implement Execute/Undo for the command pattern.
#pragma once
#include <string>

class Command {
public:
    virtual ~Command()                           = default;
    virtual void        Execute()                = 0;
    virtual void        Undo()                   = 0;
    virtual std::string GetDescription() const   = 0;
};
