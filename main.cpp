#define _WIN32_WINNT 0x0A00
#include "httplib.h"
#include <iostream>
#include "sqlite3.h"
static int callback(void* dados, int colunas, char** valores, char** nomes)
{
   std::string* texto =
   (std::string*)dados;
   for (int i = 0; i < colunas; i++) {
    *texto += std::string(nomes[i])
    + ":" + (valores[i] ? valores[i] : "vazio") +"|";
   }
   *texto += "\n";
   return 0;
}
int main() {
    httplib::Server svr;
sqlite3* db;
int resultado =
sqlite3_open("gastos.db", &db);
if (resultado != SQLITE_OK) {
    std::cerr << "Erro ao abrir o banco de dados!" << std::endl;
    return 1;
}
int resultado_tabela = sqlite3_exec(db, "create table if not exists categorias (id integer primary key autoincrement, nome text);", nullptr, nullptr, nullptr);
if (resultado_tabela != SQLITE_OK) {
    std::cerr << "erro ao criar tabela categorias!" << std::endl;
    return 1;
}
int resultado_gastos = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS gastos (id INTEGER PRIMARY KEY AUTOINCREMENT, descricao TEXT, valor REAL, data TEXT, categoria_id INTEGER, FOREIGN KEY (categoria_id) REFERENCES categorias(id));", nullptr, nullptr, nullptr);
if(resultado_gastos != SQLITE_OK)
{
std::cerr << "erro ao criar tabela gasto!" << std::endl;
return 1;
}

    svr.Get("/ola", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Servidor funcionando!", "text/plain");
    });
svr.Get("/gastos", [db](const
 httplib::Request& req,
 httplib::Response& res) {
 std::string resultado;
 char* erro = nullptr;

  int r = sqlite3_exec(db, "select * from gastos;", callback, &resultado,
 &erro);
 if (r != SQLITE_OK)
     {
         res.status = 500;
     res.set_content("erro ao buscar gastos", "text/plain; charset=utf-8");
     sqlite3_free(erro);
     return;

     }

 if (resultado.empty()) {
    resultado = "nenhum gasto cadastrado.";
 }
 res.set_content(resultado, "text/plain; charset=utf-8");
});
svr. Post("/categorias", [db](const
httplib::Request& req,
httplib::Response res) {
  std::cout << "rota post chamada\n";
  if (!req.has_param("nome")) {
        res.status = 400;
        res.set_content("faltou o parametro nome", "text/plain; charset=utf-8");
        return;
  }
  std::string nome =
  req.get_param_value("nome");

  sqlite3_stmt* stmt = nullptr;
  int r = sqlite3_prepare_v2(db,
                             "insert into categorias (nome) values(?);", -1, &stmt, nullptr);
                             if  (r != SQLITE_OK) {
                                res.status = 500;
                                res.set_content("erro ao preparar o comando", "text/plain;charset=utf-8");
                                               return;
                             }
   sqlite3_bind_text(stmt, 1,
    nome.c_str(), -1, SQLITE_TRANSIENT);
    r = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    std::cout << "resultado do step: " << r << "\n";

    if (r != SQLITE_DONE) {
        res.status = 500;
        res.set_content("erro ao cadastrar categoria", "text/plain;charset=utf-8");
                        return;
    }
    std::cout << "chegou no 201\n";
    res.status = 201;
    res.set_content("categoria cadastrada!", "text/plain;charset=utf-8");
    });
svr.Get("/categorias", [db](const httplib::Request& req, httplib::Response& res) {
        std::string resultado;
        char* erro = nullptr;
        int r = sqlite3_exec(db, "select * from categorias;", callback, &resultado, &erro);
        if (r != SQLITE_OK) {
            res.status = 500;
            res.set_content("erro ao buscar categorias", "text/plain; charset=utf-8");
            sqlite3_free(erro);
            return;
        }
        if (resultado.empty()) {
            resultado = "nenhuma categoria cadastrada.";
        }
        res.set_content(resultado, "text/plain; charset=utf-8");
        });
    std::cout << "Servidor v2 rodando em http://localhost:8080\n";
    svr.listen("localhost", 8080);
}
