#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include <iostream>

#include <csignal>
#include <unistd.h>
#include "db/alphadb.h"
#include "sql.h"
#include "conn.h"
#include "server.h"
#include "dispatcher.h"
#include "raft_server.h"

const std::string DEFAULT_DB_NAME{"default"};

int main(int argc, char* argv[])
{
    if (argc == 1) {
        std::cout << intro();
        std::cout << usage();
    }
    else if (argc == 2)
    {
        if (argv[1] == "server")
            start_server();
        else
            start_client();
    }
    else {
        std::cerr << "error" << std::endl;
    }
}

void start_server()
{
    int id;
    short port;
    // register signal to do clean up function

    dispatcher* d= new dispatcher(4);

    alphadb* db = new alphadb();

    server* s1 = new server(d, 80);
    s1->start();

    raft_server* s2 = new raft_server(d, port, id);
    s2->start();

    s2->set_apply_func([db](struct log l) {
        return true;
    });

    s1->set_callback([db, s2](conn* c) {
        c->on_read([c, db, s2]{
            // receive command from client
            buffer* buf = c->read_buffer();
            int size = 10;
            char sql[size];
            // unmarshall it
            buf->get(sql, size);
            std::string reply;
            bool ok = db->serve_command(std::string(sql), reply);
            struct log l;
            s2->replicate(l);
            c->send(reply.c_str());
        });
    });
    
    d->loop();
}

void start_client()
{
    int pipe_fd[2];
    if (pipe(pipe_fd) < 0) {
        std::cerr << "pipe error" << std::endl;
        exit(-1);
    }

    dispatcher* d;
    channel* chan;
    conn* c;
    bool quit = false;
    std::string instruction;
    std::vector<std::pair<std::string, short>> servers{std::make_pair("localhost", 8081), std::make_pair("localhost", 8082), std::make_pair("localhost", 8083)};

    pid_t pid = fork();
    if (pid > 0) {
        close(pipe_fd[1]);
        d = new dispatcher(4);
        c = new conn(servers[0], d->poller());
        chan = new channel(pipe_fd[0], d->poller());
        c->on_read([&, c]() {
            buffer* buf = c->read_buffer();
            char reply[1024];
            buf->get(reply, 1024);
            std::cout << reply << std::endl;
        });
        chan->on_read([&]{
            char buff[4096];
            int size = read(chan->fd(), buff, 4096);
            
            if (size != 0) {
                buffer* buf = c->write_buffer();
                buf->put(buff, size);
                c->send_output();
            }
        });
        d->loop();
    }
    else {
        // the child process is responsible for writing
        // so close the reading end first
        close(pipe_fd[0]);
        std::cout << "Hello world!" << std::endl;

        while (!quit)
        {
            std::cout << ">";
            std::cin >> instruction;
        
            if (instruction == "quit") {
                quit = true;
                continue;
            }

            parser_result result;
            bool success = sql::parse(instruction, result);
            if (!success)
                std::cout << "INVALID INPUT, please try again." << std::endl;
            
            // TODO marshall the command
            std::string msg;
            
            if (write(pipe_fd[1], msg.c_str(), msg.length()) < 0)
                exit(-1);
            
            c->send(instruction.c_str());
        }

        std::cout << "See you!" << std::endl;
    }

    if (pid)
        close(pipe_fd[0]);
    else
        close(pipe_fd[1]); 
}

std::string intro()
{
    std::ostringstream oss;
    return oss.str();
}

std::string usage()
{
    std::ostringstream oss;
    return oss.str();
}