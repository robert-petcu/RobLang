#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

std::ifstream get_code("main.txt");
std::ifstream fin("input.txt");
std::ofstream fout("output.txt");


std::string current_line;
bool skip_next_else = false;

std::unordered_map<std::string, std::string> variable_types;

std::unordered_map<std::string, int> memory_int;
std::unordered_map<std::string, bool> memory_bool;
std::unordered_map<std::string, double> memory_double;
std::unordered_map<std::string, std::string> memory_string;

std::unordered_map<std::string, std::vector<int>> memory_vector_int;
std::unordered_map<std::string, std::vector<bool>> memory_vector_bool;
std::unordered_map<std::string, std::vector<double>> memory_vector_double;
std::unordered_map<std::string, std::vector<std::string>> memory_vector_string;

std::unordered_map<std::string, std::vector<std::vector<int>>> memory_matrix_int;
std::unordered_map<std::string, std::vector<std::vector<bool>>> memory_matrix_bool;
std::unordered_map<std::string, std::vector<std::vector<double>>> memory_matrix_double;
std::unordered_map<std::string, std::vector<std::vector<std::string>>> memory_matrix_string;

std::unordered_set<std::string> valid_types = {"int", "bool", "double", "string"};
std::unordered_set<std::string> valid_vector_types = {"vector_int", "vector_bool", "vector_double", "vector_string"};
std::unordered_set<std::string> valid_matrix_types = {"matrix_int", "matrix_bool", "matrix_double", "matrix_string"};

void execute_current_line(const std::vector<std::string>& source, int& line_idx);
std::vector<std::string> read_block(const std::vector<std::string>& source, int& line_idx);
void execute_block(const std::vector<std::string>& instructions);

double evaluate_logic(const std::string& expr, int& pos);

void skip_spaces(int &i) {
    while (i < current_line.size() && current_line[i] == ' ') i++;
}

void skip_spaces_expr(const std::string& expr, int &i) {
    while (i < expr.size() && expr[i] == ' ') i++;
}

double evaluate_factor(const std::string& expr, int& pos) {
    skip_spaces_expr(expr, pos);
    if (pos >= expr.size()) return 0;

    if (expr[pos] == '(') {
        pos++;
        double val = evaluate_logic(expr, pos);
        skip_spaces_expr(expr, pos);
        if (pos < expr.size() && expr[pos] == ')') pos++;
        return val;
    }

    if (isdigit(expr[pos]) || expr[pos] == '-') {
        double val = 0;
        int sign = 1;
        if (expr[pos] == '-') { sign = -1; pos++; }
        while (pos < expr.size() && isdigit(expr[pos])) {
            val = val * 10 + (expr[pos] - '0');
            pos++;
        }
        if (pos < expr.size() && expr[pos] == '.') {
            pos++;
            double fraction = 10.0;
            while (pos < expr.size() && isdigit(expr[pos])) {
                val += (expr[pos] - '0') / fraction;
                fraction *= 10.0;
                pos++;
            }
        }
        return val * sign;
    }

    std::string var_name;
    while (pos < expr.size() && (isalnum(expr[pos]) || expr[pos] == '_')) {
        var_name += expr[pos++];
    }

    skip_spaces_expr(expr, pos);
    if (pos < expr.size() && expr[pos] == '[') {
        pos++;
        int idx_i = (int)evaluate_logic(expr, pos);
        skip_spaces_expr(expr, pos);
        if (pos < expr.size() && expr[pos] == ']') pos++;

        skip_spaces_expr(expr, pos);

        if (pos < expr.size() && expr[pos] == '[') {
            pos++;
            int idx_j = (int)evaluate_logic(expr, pos);
            skip_spaces_expr(expr, pos);
            if (pos < expr.size() && expr[pos] == ']') pos++;

            skip_spaces_expr(expr, pos);

            if (variable_types[var_name] == "matrix_int") return memory_matrix_int[var_name][idx_i][idx_j];
            if (variable_types[var_name] == "matrix_bool") return memory_matrix_bool[var_name][idx_i][idx_j];
            if (variable_types[var_name] == "matrix_double") return memory_matrix_double[var_name][idx_i][idx_j];
        }

        if (variable_types[var_name] == "vector_int") return memory_vector_int[var_name][idx_i];
        if (variable_types[var_name] == "vector_bool") return memory_vector_bool[var_name][idx_i];
        if (variable_types[var_name] == "vector_double") return memory_vector_double[var_name][idx_i];
    }

    if (variable_types[var_name] == "int") return memory_int[var_name];
    if (variable_types[var_name] == "double") return memory_double[var_name];
    if (variable_types[var_name] == "bool") return memory_bool[var_name];

    return 0;
}

double evaluate_term(const std::string& expr, int& pos) {
    double left = evaluate_factor(expr, pos);
    while (true) {
        skip_spaces_expr(expr, pos);
        if (pos >= expr.size()) break;
        if (expr[pos] == '*') {
            pos++; left *= evaluate_factor(expr, pos);
        } else if (expr[pos] == '/') {
            pos++; left /= evaluate_factor(expr, pos);
        } else if (expr[pos] == '%') {
            pos++; left = (int)left % (int)evaluate_factor(expr, pos);
        } else {
            break;
        }
    }
    return left;
}

double evaluate_math(const std::string& expr, int& pos) {
    double left = evaluate_term(expr, pos);
    while (true) {
        skip_spaces_expr(expr, pos);
        if (pos >= expr.size()) break;
        if (expr[pos] == '+') {
            pos++; left += evaluate_term(expr, pos);
        } else if (expr[pos] == '-') {
            pos++; left -= evaluate_term(expr, pos);
        } else {
            break;
        }
    }
    return left;
}

double evaluate_comparison(const std::string& expr, int& pos) {
    double left = evaluate_math(expr, pos);
    skip_spaces_expr(expr, pos);
    if (pos + 1 < expr.size() && expr[pos] == '=' && expr[pos+1] == '=') { pos += 2; return left == evaluate_math(expr, pos); }
    if (pos + 1 < expr.size() && expr[pos] == '!' && expr[pos+1] == '=') { pos += 2; return left != evaluate_math(expr, pos); }
    if (pos + 1 < expr.size() && expr[pos] == '<' && expr[pos+1] == '=') { pos += 2; return left <= evaluate_math(expr, pos); }
    if (pos + 1 < expr.size() && expr[pos] == '>' && expr[pos+1] == '=') { pos += 2; return left >= evaluate_math(expr, pos); }
    if (pos < expr.size() && expr[pos] == '<') { pos++; return left < evaluate_math(expr, pos); }
    if (pos < expr.size() && expr[pos] == '>') { pos++; return left > evaluate_math(expr, pos); }
    return left;
}

double evaluate_logic(const std::string& expr, int& pos) {
    double left = evaluate_comparison(expr, pos);
    while (true) {
        skip_spaces_expr(expr, pos);
        if (pos + 1 < expr.size() && expr[pos] == '&' && expr[pos+1] == '&') {
            pos += 2; left = (left != 0 && evaluate_comparison(expr, pos) != 0);
        } else if (pos + 1 < expr.size() && expr[pos] == '|' && expr[pos+1] == '|') {
            pos += 2; left = (left != 0 || evaluate_comparison(expr, pos) != 0);
        } else {
            break;
        }
    }
    return left;
}

void get_command(std::string &command, int &i) {
    skip_spaces(i);
    while (i < current_line.size() && current_line[i] != ' ') {
        command += current_line[i++];
    }
}

void declare_type(const std::string& command, int &i) {
    skip_spaces(i);
    std::string name;
    while (i < current_line.size() && (isalnum(current_line[i]) || current_line[i] == '_')) {
        name += current_line[i++];
    }

    skip_spaces(i);

    bool is_vector = false, is_matrix = false;
    std::string size_n, size_m;
    if (i < current_line.size() && current_line[i] == '[') {
        is_vector = true; i++;
        while (i < current_line.size() && current_line[i] != ']') size_n += current_line[i++];
        if (i < current_line.size() && current_line[i] == ']') i++;
    }
    skip_spaces(i);
    if (i < current_line.size() && current_line[i] == '[') {
        is_matrix = true; is_vector = false; i++;
        while (i < current_line.size() && current_line[i] != ']') size_m += current_line[i++];
        if (i < current_line.size() && current_line[i] == ']') i++;
    }

    if (is_matrix) {
        int pn = 0, pm = 0;
        int n = (int)evaluate_logic(size_n, pn), m = (int)evaluate_logic(size_m, pm);
        variable_types[name] = "matrix_" + command;
        if (command == "int") memory_matrix_int[name].resize(n, std::vector<int>(m, 0));
        else if (command == "bool") memory_matrix_bool[name].resize(n, std::vector<bool>(m, false));
        else if (command == "double") memory_matrix_double[name].resize(n, std::vector<double>(m, 0.0));
        else if (command == "string") memory_matrix_string[name].resize(n, std::vector<std::string>(m, ""));
    }
    else if (is_vector) {
        int pn = 0;
        int n = (int)evaluate_logic(size_n, pn);
        variable_types[name] = "vector_" + command;
        if (command == "int") memory_vector_int[name].resize(n, 0);
        else if (command == "bool") memory_vector_bool[name].resize(n, false);
        else if (command == "double") memory_vector_double[name].resize(n, 0.0);
        else if (command == "string") memory_vector_string[name].resize(n, "");
    }
    else {
        variable_types[name] = command;
        double val = 0;
        std::string str_val = "";

        skip_spaces(i);
        if (i < current_line.size() && current_line[i] == ':') {
            i++;
            skip_spaces(i);

            std::string expr;
            while(i < current_line.size() && current_line[i] != ';') expr += current_line[i++];

            if (command == "string") {
                if (expr.size() >= 2 && expr.front() == '"') {
                    int idx = 1;
                    while(idx < expr.size() && expr[idx] != '"') str_val += expr[idx++];
                } else str_val = expr;
            } else {
                int p = 0;
                val = evaluate_logic(expr, p);
            }
        }

        if (command == "int") memory_int[name] = (int)val;
        else if (command == "bool") memory_bool[name] = (val != 0);
        else if (command == "double") memory_double[name] = val;
        else if (command == "string") memory_string[name] = str_val;
    }
}

void read(int &i) {
    skip_spaces(i);
    std::string name;
    while (i < current_line.size() && current_line[i] != ' ' && current_line[i] != '[' && current_line[i] != ':') {
        name += current_line[i++];
    }

    int position1 = 0;
    int position2 = 0;

    if (i < current_line.size() && current_line[i] == '[') {
        i++;
        std::string idx_expr;
        while(i < current_line.size() && current_line[i] != ']') idx_expr += current_line[i++];
        i++;
        int p_idx = 0;
        position1 = (int)evaluate_logic(idx_expr, p_idx);
    }

    skip_spaces(i);

    if (i < current_line.size() && current_line[i] == '[') {
        i++;
        std::string idx_expr;
        while(i < current_line.size() && current_line[i] != ']') idx_expr += current_line[i++];
        i++;
        int p_idx = 0;
        position2 = (int)evaluate_logic(idx_expr, p_idx);
    }

    std::string full_content;
    fin >> full_content;

    if (variable_types[name] == "string" || variable_types[name] == "vector_string" || variable_types[name] == "matrix_string") {
        if (variable_types[name] == "string") memory_string[name] = full_content;
        else if (variable_types[name] == "vector_string") memory_vector_string[name][position1] = full_content;
        else memory_matrix_string[name][position1][position2] = full_content;
    } else {
        std::string numeric_part;
        int j = 0;

        if (j < full_content.size() && full_content[j] == '-') {
            numeric_part += full_content[j++];
        }

        bool decimal_point_found = false;
        while (j < full_content.size()) {
            if (isdigit(full_content[j])) {
                numeric_part += full_content[j];
            } else if (full_content[j] == '.' && !decimal_point_found) {
                numeric_part += full_content[j];
                decimal_point_found = true;
            } else {
                break;
            }
            j++;
        }

        if (numeric_part.empty() || numeric_part == "-") numeric_part = "0";

        int expr_pos = 0;
        double val = evaluate_logic(numeric_part, expr_pos);

        if (variable_types[name] == "int") memory_int[name] = (int)val;
        else if (variable_types[name] == "bool") memory_bool[name] = (val != 0);
        else if (variable_types[name] == "double") memory_double[name] = val;
        else if (variable_types[name] == "vector_int") memory_vector_int[name][position1] = (int)val;
        else if (variable_types[name] == "vector_bool") memory_vector_bool[name][position1] = (val != 0);
        else if (variable_types[name] == "vector_double") memory_vector_double[name][position1] = val;
        else if (variable_types[name] == "matrix_int") memory_matrix_int[name][position1][position2] = (int)val;
        else if (variable_types[name] == "matrix_bool") memory_matrix_bool[name][position1][position2] = (val != 0);
        else if (variable_types[name] == "matrix_double") memory_matrix_double[name][position1][position2] = val;
    }
}

void assign(int &i) {
    skip_spaces(i);
    std::string name;
    while (i < current_line.size() && current_line[i] != ' ' && current_line[i] != '[' && current_line[i] != ':') {
        name += current_line[i++];
    }

    int position1 = 0;
    int position2 = 0;

    if (i < current_line.size() && current_line[i] == '[') {
        i++;
        std::string idx_expr;
        while(i < current_line.size() && current_line[i] != ']') idx_expr += current_line[i++];
        i++;
        int p_idx = 0;
        position1 = (int)evaluate_logic(idx_expr, p_idx);
    }

    skip_spaces(i);

    if (i < current_line.size() && current_line[i] == '[') {
        i++;
        std::string idx_expr;
        while(i < current_line.size() && current_line[i] != ']') idx_expr += current_line[i++];
        i++;
        int p_idx = 0;
        position2 = (int)evaluate_logic(idx_expr, p_idx);
    }

    skip_spaces(i);

    if (i < current_line.size() && current_line[i] == ':') i++;
    skip_spaces(i);

    std::string content = current_line.substr(i);

    if (variable_types[name] == "string" || variable_types[name] == "vector_string" || variable_types[name] == "matrix_string") {
        std::string final_str;
        if (content.size() >= 2 && content.front() == '"') {
            int j = 1;
            while (j < content.size() && content[j] != '"') {
                final_str += content[j++];
            }
        } else {
            final_str = content;
        }

        if (variable_types[name] == "string") memory_string[name] = final_str;
        else if (variable_types[name] == "vector_string") memory_vector_string[name][position1] = final_str;
        else memory_matrix_string[name][position1][position2] = final_str;
    } else {
        int expr_pos = 0;
        double val = evaluate_logic(content, expr_pos);

        if (variable_types[name] == "int") memory_int[name] = (int)val;
        else if (variable_types[name] == "bool") memory_bool[name] = (val != 0);
        else if (variable_types[name] == "double") memory_double[name] = val;
        else if (variable_types[name] == "vector_int") memory_vector_int[name][position1] = (int)val;
        else if (variable_types[name] == "vector_bool") memory_vector_bool[name][position1] = (val != 0);
        else if (variable_types[name] == "vector_double") memory_vector_double[name][position1] = val;
        else if (variable_types[name] == "matrix_int") memory_matrix_int[name][position1][position2] = (int)val;
        else if (variable_types[name] == "matrix_bool") memory_matrix_bool[name][position1][position2] = (val != 0);
        else if (variable_types[name] == "matrix_double") memory_matrix_double[name][position1][position2] = val;
    }
}

void print(int &i) {
    skip_spaces(i);
    std::string line_remainder = current_line.substr(i);

    if (line_remainder[0] == '"') {
        if (line_remainder == "\"endl\"") fout << '\n';
        else {
            for (int j = 1; j < line_remainder.size() && line_remainder[j] != '"'; j++)
                fout << line_remainder[j];
        }
        return;
    }

    int pos = 0;
    std::string name;
    while (pos < line_remainder.size() && (isalnum(line_remainder[pos]) || line_remainder[pos] == '_')) {
        name += line_remainder[pos++];
    }

    int idx1 = 0, idx2 = 0;
    bool has_idx1 = false, has_idx2 = false;

    skip_spaces_expr(line_remainder, pos);
    if (pos < line_remainder.size() && line_remainder[pos] == '[') {
        pos++;
        idx1 = (int)evaluate_logic(line_remainder, pos);
        if (line_remainder[pos] == ']') pos++;
        has_idx1 = true;
    }

    skip_spaces_expr(line_remainder, pos);
    if (pos < line_remainder.size() && line_remainder[pos] == '[') {
        pos++;
        idx2 = (int)evaluate_logic(line_remainder, pos);
        if (line_remainder[pos] == ']') pos++;
        has_idx2 = true;
    }

    std::string type = variable_types[name];

    if (type == "string") fout << memory_string[name];
    else if (type == "vector_string") fout << memory_vector_string[name][idx1];
    else if (type == "matrix_string") fout << memory_matrix_string[name][idx1][idx2];
    else {
        int start_pos = 0;
        fout << evaluate_logic(line_remainder, start_pos);
    }
}
std::vector<std::string> read_block(const std::vector<std::string>& source, int& line_idx) {
    std::vector<std::string> block;
    int nested = 0;
    while (line_idx < source.size()) {
        std::string line = source[line_idx++];

        int i = 0;
        while (i < line.size() && line[i] == ' ') i++;

        if (i < line.size() && line[i] == '}') {
            if (nested == 0) {
                line_idx--;
                break;
            } else {
                nested--;
            }
        }

        block.push_back(line);

        for (char c : line) {
            if (c == '{') nested++;
        }
    }
    return block;
}

void execute_block(const std::vector<std::string>& instructions) {
    std::string saved_line = current_line;
    int idx = 0;
    while (idx < instructions.size()) {
        current_line = instructions[idx++];
        if (current_line.empty()) continue;
        execute_current_line(instructions, idx);
    }
    current_line = saved_line;
}

void process_if(int &i, const std::vector<std::string>& source, int& line_idx) {
    skip_spaces(i);
    std::string condition = current_line.substr(i);
    int pos = 0;
    bool is_true = (evaluate_logic(condition, pos) != 0);

    std::vector<std::string> block = read_block(source, line_idx);

    if (is_true) {
        execute_block(block);
        skip_next_else = true;
    } else {
        skip_next_else = false;
    }
}

void process_else_if(int &i, const std::vector<std::string>& source, int& line_idx) {
    skip_spaces(i);
    std::string next_word;
    int temp_i = i;
    while (temp_i < current_line.size() && current_line[temp_i] != ' ' && current_line[temp_i] != '{') {
        next_word += current_line[temp_i++];
    }

    if (next_word == "if") {
        i = temp_i;
        skip_spaces(i);
        std::string condition = current_line.substr(i);
        int pos = 0;
        bool is_true = (evaluate_logic(condition, pos) != 0);
        std::vector<std::string> block = read_block(source, line_idx);

        if (!skip_next_else && is_true) {
            execute_block(block);
            skip_next_else = true;
        }
    } else {
        std::vector<std::string> block = read_block(source, line_idx);
        if (!skip_next_else) {
            execute_block(block);
        }
        skip_next_else = false;
    }
}

void process_while(int &i, const std::vector<std::string>& source, int& line_idx) {
    skip_spaces(i);
    std::string condition = current_line.substr(i);
    std::vector<std::string> block = read_block(source, line_idx);

    int pos = 0;
    while (evaluate_logic(condition, pos) != 0) {
        execute_block(block);
        pos = 0;
    }
}

void for_loop(int &i, const std::vector<std::string>& source, int& line_idx) {
    skip_spaces(i);

    std::string decl_expr, cond_expr, update_expr;

    while (i < current_line.size() && current_line[i] != ';') decl_expr += current_line[i++];
    if (i < current_line.size() && current_line[i] == ';') i++;

    while (i < current_line.size() && current_line[i] != ';') cond_expr += current_line[i++];
    if (i < current_line.size() && current_line[i] == ';') i++;

    while (i < current_line.size() && current_line[i] != '{') update_expr += current_line[i++];

    std::vector<std::string> block = read_block(source, line_idx);

    std::string saved_line = current_line;
    int dummy_idx = 0;

    current_line = decl_expr;
    execute_current_line({decl_expr}, dummy_idx);

    while (true) {
        int pos = 0;
        if (evaluate_logic(cond_expr, pos) == 0) break;

        execute_block(block);

        current_line = update_expr;
        int temp_idx = 0;
        execute_current_line({update_expr}, temp_idx);
    }

    current_line = saved_line;

    int p = 0;
    while (p < decl_expr.size() && decl_expr[p] == ' ') p++;

    std::string var_type;
    while (p < decl_expr.size() && decl_expr[p] != ' ' && decl_expr[p] != ';') var_type += decl_expr[p++];

    while (p < decl_expr.size() && decl_expr[p] == ' ') p++;

    std::string var_name;
    while (p < decl_expr.size() && decl_expr[p] != ' ' && decl_expr[p] != ';') var_name += decl_expr[p++];

    if (!var_name.empty()) {
        if (var_type == "int") memory_int.erase(var_name);
        else if (var_type == "double") memory_double.erase(var_name);
        else if (var_type == "bool") memory_bool.erase(var_name);
        else if (var_type == "string") memory_string.erase(var_name);
        variable_types.erase(var_name);
    }
}
void execute_current_line(const std::vector<std::string>& source, int& line_idx) {
    int i = 0;
    skip_spaces(i);

    if (i + 1 < current_line.size() && current_line[i] == '/' && current_line[i + 1] == '/') {
        return;
    }

    if (i < current_line.size() && current_line[i] == '}') {
        i++;
        skip_spaces(i);
        if (i >= current_line.size()) return;
    }

    std::string command;
    get_command(command, i);

    if (valid_types.count(command)) declare_type(command, i);
    else if (command == "read") read(i);
    else if (command == "assign") assign(i);
    else if (command == "print") print(i);
    else if (command == "for") for_loop(i, source, line_idx);
    else if (command == "while") process_while(i, source, line_idx);
    else if (command == "if") process_if(i, source, line_idx);
    else if (command == "else") process_else_if(i, source, line_idx);
}

int main() {
    std::vector<std::string> all_lines;
    std::string line;

    while (std::getline(get_code, line)) {
        all_lines.push_back(line);
    }

    execute_block(all_lines);

    return 0;
}
