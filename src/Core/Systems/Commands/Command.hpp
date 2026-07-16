// Abstract editor command; subclasses implement Execute/Undo for the command pattern.
#pragma once
#include <string>

class Command {
public:
    virtual ~Command()                                       = default;
    Command(Command const&)                                  = delete;
    Command& operator=(Command const&)                       = delete;
    Command(Command&&)                                       = default;
    Command& operator=(Command&&)                            = default;
    virtual void Execute()                                   = 0;
    virtual void Undo()                                      = 0;
    [[nodiscard]] virtual std::string GetDescription() const = 0;

protected:
    Command() = default;
};
