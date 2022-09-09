#include "SignalHandler.hh"

#include <csignal>
#include <iostream>

using namespace Tools;

// initialize all the static stuff
SignalHandler *SignalHandler::m_instance = 0;
std::map<int, std::set<EventHandler *>> SignalHandler::m_signal_handlers;
std::map<int, sighandler_t> SignalHandler::m_signal_old_handler;

SignalHandler *SignalHandler::handler()
{
    // make an object if needed
    if (m_instance == 0)
        m_instance = new SignalHandler();
    return m_instance;
}

void SignalHandler::register_handler(int signum, EventHandler *eh)
{
    // check if the signal was not bound to anything
    bool was_empty = m_signal_handlers[signum].empty();

    // now store the new handler
    m_signal_handlers[signum].insert(eh);

    // register the dispatcher to handle this signal
    struct sigaction sa, old_sa;
    sa.sa_handler = SignalHandler::dispatcher;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(signum, &sa, &old_sa);

    // if the signal was unbound by us, store where is was bound to before
    if (was_empty)
        m_signal_old_handler[signum] = old_sa.sa_handler;
}

void SignalHandler::remove_handler(int signum, EventHandler *eh)
{
    // remove the requesting handler
    size_t removed = m_signal_handlers[signum].erase(eh);
    // if none was removed, just return
    if (removed == 0)
        return;

    // if there is no handler left, reset the signal to where it was before
    if (m_signal_handlers[signum].empty())
        signal(signum, m_signal_old_handler[signum]);
}

void SignalHandler::dispatcher(int signum)
{
    std::cerr << "Signal: " << signum << std::endl;
    // get the set of handlers for that specific signal
    std::set<EventHandler *> &handlers = m_signal_handlers[signum];
    // now loop and call all handlers
    for (std::set<EventHandler *>::iterator handler = handlers.begin(); handler != handlers.end(); ++handler)
    {
        std::cerr << "Calling handler." << std::endl;
        (*handler)->handle_signal(signum);
        std::cerr << "Handler returned." << std::endl;
    }
    // re-raise for proper termination
    std::cerr << "Re-raising." << std::endl;
    raise(signum);
    std::cerr << "Re-raised." << std::endl;
}
