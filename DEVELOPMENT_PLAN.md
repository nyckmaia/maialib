# Plano de Desenvolvimento e Melhorias - Maialib

> **Data do Plano:** 19 de Novembro de 2025
> **Versão Atual do Projeto:** v1.10.0
> **Status do Projeto:** Produção Ativa

---

## Sumário Executivo

### Avaliação Geral do Projeto

O **Maialib** é uma biblioteca de análise musical multiplataforma bem arquitetada, com núcleo em C++17 (maiacore) e bindings Python via pybind11. O projeto demonstra alta qualidade no código C++, documentação excelente e tutoriais bem estruturados.

**Pontuação de Saúde do Projeto: 8.5/10**

### Métricas Principais

| Métrica                        | Valor               | Avaliação           |
| ------------------------------ | ------------------- | ------------------- |
| **Cobertura de Testes C++**    | ~44% (9.761 linhas) | ✅ Excelente        |
| **Cobertura de Testes Python** | ~5% (737 linhas)    | ⚠️ Crítico          |
| **Qualidade do Código C++**    | Alta                | ✅ Excelente        |
| **Qualidade do Código Python** | Boa                 | 🟡 Melhorar         |
| **Documentação**               | Completa            | ✅ Excelente        |
| **Tutoriais**                  | 25 notebooks        | ✅ Excelente        |
| **Sistema de Build**           | Funcional           | 🟡 Falta formatação |
| **Arquitetura**                | Sólida              | ✅ Bom              |

### Top 3 Prioridades Imediatas

1. **🔴 CRÍTICO:** Ampliar cobertura de testes Python (de 5% para 80%+)
2. **🟠 ALTO:** Adicionar automação de formatação de código (`make format`)
3. **🟠 ALTO:** Corrigir bugs críticos no código C++ (copy constructor, debug statements)

---

## 1. Formatação e Estilo de Código

### 1.1 Estado Atual

#### C++ (maiacore/)

- ✅ Arquivo `.clang-format` configurado (Google style, 100 chars, 4 espaços)
- ✅ Código geralmente bem formatado
- ❌ **Problema:** Não existe target `make format` para aplicar formatação automaticamente
- ❌ **Problema:** Formatação manual e inconsistente

#### Python (maialib/)

- ❌ Sem configuração de formatação (black, isort)
- ❌ Sem arquivo `pyproject.toml` com configurações de ferramentas
- ❌ Comentários em português misturados com inglês
- 🟡 Código segue PEP 8 manualmente

### 1.2 Plano de Ação

#### Fase 1: Configurar Ferramentas de Formatação Python

**Criar `pyproject.toml` com configurações:**

```toml
[tool.ruff]
# Ruff configuration - replaces both black and isort
line-length = 100
target-version = "py38"

# Enable specific rule sets
select = [
    "E",   # pycodestyle errors
    "W",   # pycodestyle warnings
    "F",   # pyflakes
    "I",   # isort (import sorting)
    "N",   # pep8-naming
    "UP",  # pyupgrade
    "B",   # flake8-bugbear
    "C4",  # flake8-comprehensions
    "SIM", # flake8-simplify
]

# Ignore specific rules
ignore = [
    "E501",  # Line too long (handled by formatter)
]

# Exclude directories
exclude = [
    ".eggs",
    ".git",
    ".hg",
    ".mypy_cache",
    ".tox",
    ".venv",
    "build",
    "dist",
    "__pycache__",
]

# Format configuration (Ruff's formatter replaces black)
[tool.ruff.format]
quote-style = "double"
indent-style = "space"
line-ending = "auto"

# Import sorting configuration (replaces isort)
[tool.ruff.lint.isort]
known-first-party = ["maialib", "maiacore", "maiapy"]
section-order = ["future", "standard-library", "third-party", "first-party", "local-folder"]
```

#### Fase 2: Adicionar Targets ao Makefile

**Adicionar os seguintes targets:**

```makefile
# Code Formatting
format-cpp:
	@echo "Formatting C++ code..."
	@find maiacore/include/maiacore -name "*.h" -exec clang-format -i {} +
	@find maiacore/src/maiacore -name "*.cpp" -o -name "*.h" -exec clang-format -i {} +
	@find tests-cpp/src -name "*.cpp" -o -name "*.h" -exec clang-format -i {} +
	@echo "C++ formatting complete."

format-python:
	@echo "Formatting Python code with Ruff..."
	@ruff format maialib/ test/ scripts/
	@echo "Python formatting complete."

format: format-cpp format-python
	@echo "All code formatted successfully."

# Linting
lint-python:
	@echo "Linting Python code with Ruff..."
	@ruff check maialib/ test/ scripts/
	@echo "Python linting complete."

lint-python-fix:
	@echo "Linting and auto-fixing Python code with Ruff..."
	@ruff check --fix maialib/ test/ scripts/
	@echo "Python linting and fixes complete."

validate: lint-python  # Adicionar ao validate existente
```

#### Fase 3: Configurar Pre-commit Hooks

**Criar `.pre-commit-config.yaml`:**

```yaml
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.5.0
    hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer
      - id: check-yaml
      - id: check-added-large-files
      - id: mixed-line-ending

  # Ruff - replaces black, isort, and multiple flake8 plugins
  - repo: https://github.com/astral-sh/ruff-pre-commit
    rev: v0.8.4 # Use the latest version
    hooks:
      # Run the linter
      - id: ruff
        args: [--fix, --exit-non-zero-on-fix]
      # Run the formatter
      - id: ruff-format

  # C++ formatting
  - repo: local
    hooks:
      - id: clang-format
        name: clang-format
        entry: clang-format
        language: system
        types: [c++]
        args: [-i]
```

#### Fase 4: Atualizar Documentação

**Adicionar ao `CONTRIBUTING.md`:**

````markdown
## Formatação de Código

### Antes de Submeter um Pull Request

1. Execute a formatação automática:
   ```bash
   make format
   ```
````

2. Verifique se não há erros de linting:

```bash
make validate
```

### Configuração de Pre-commit Hooks (Opcional mas Recomendado)

```bash
pip install pre-commit
pre-commit install
```

Os hooks serão executados automaticamente antes de cada commit.

#### Fase 5: Padronizar Comentários para Inglês

**Arquivos com comentários em português:**

- `maialib/maiapy/plots.py` (linhas 290, 375, 551-552)
- `maialib/maiapy/sethares_dissonance.py` (linhas 375, 380, 384, 406)

**Ação:** Traduzir todos os comentários para inglês durante a formatação inicial.

### 1.3 Estimativa de Esforço

| Tarefa                           | Tempo         | Prioridade |
| -------------------------------- | ------------- | ---------- |
| Criar pyproject.toml             | 30 min        | ALTA       |
| Adicionar targets ao Makefile    | 1 hora        | ALTA       |
| Criar .pre-commit-config.yaml    | 30 min        | MÉDIA      |
| Formatar todo código Python      | 1 hora        | ALTA       |
| Traduzir comentários para inglês | 2 horas       | MÉDIA      |
| Atualizar CONTRIBUTING.md        | 30 min        | MÉDIA      |
| **TOTAL**                        | **5-6 horas** | -          |

---

## 2. Validação e Correção de Erros

### 2.1 Estado Atual do `make validate`

O comando `make validate` executa:

- **cpplint:** Verifica estilo C++ (Google Style Guide, limite 100 caracteres)
- **cppcheck:** Análise estática de código C++

### 2.2 Bugs Críticos Identificados

#### 🔴 BUG 1: Copy Constructor Incompleto (SEVERIDADE ALTA)

**Localização:** `maiacore/include/maiacore/score.h:505-516`

**Problema:**

```cpp
Score(const Score& other) {
    _title = other._title;
    _composerName = other._composerName;
    _part = other._part;
    _numParts = other._numParts;
    _numMeasures = other._numMeasures;
    _numNotes = other._numNotes;
    _isValidXML = other._isValidXML;
    _haveTypeTag = other._haveTypeTag;
    _isLoadedXML = other._isLoadedXML;
    _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
}
```

**Membros NÃO copiados:**

- `_filePath`
- `_fileName`
- `_doc` (pugi::xml_document)
- `_stackedChords`
- `_haveAnacrusisMeasure`
- `_cachedNoteEvents`
- `_isNoteEventsCached`
- `_cachedNoteEventsPerPart`
- `_isNoteEventsPerPartCached`

**Impacto:** Copiar um objeto Score cria estado inconsistente, podendo causar crashes ou comportamento inesperado.

**Solução:**

```cpp
// Opção 1: Completar o copy constructor
Score(const Score& other) {
    _title = other._title;
    _composerName = other._composerName;
    _filePath = other._filePath;
    _fileName = other._fileName;
    _part = other._part;
    _numParts = other._numParts;
    _numMeasures = other._numMeasures;
    _numNotes = other._numNotes;
    _isValidXML = other._isValidXML;
    _haveTypeTag = other._haveTypeTag;
    _isLoadedXML = other._isLoadedXML;
    _lcmDivisionsPerQuarterNote = other._lcmDivisionsPerQuarterNote;
    _stackedChords = other._stackedChords;
    _haveAnacrusisMeasure = other._haveAnacrusisMeasure;

    // Deep copy do XML document
    _doc.reset(other._doc);

    // Invalidar caches (serão reconstruídos quando necessário)
    _isNoteEventsCached = false;
    _isNoteEventsPerPartCached = false;
    _cachedNoteEvents.clear();
    _cachedNoteEventsPerPart.clear();
}

// Opção 2: Deletar o copy constructor (se cópia não for necessária)
Score(const Score& other) = delete;
```

#### 🔴 BUG 2: Assignment Operator Incompleto (SEVERIDADE ALTA)

**Localização:** `maiacore/include/maiacore/score.h:523-539`

**Problema:** Mesmo que o copy constructor, o assignment operator não copia todos os membros.

**Solução:** Aplicar a mesma correção do copy constructor.

#### 🟡 CODE SMELL 1: Ponteiro Raw em Struct (SEVERIDADE BAIXA)

**Localização:** `maiacore/include/maiacore/score.h:53`

**Problema:**

```cpp
const Note* notePtr;  // No struct NoteEvent
```

**Impacto:** Potencial dangling pointer se objetos Note forem destruídos.

**Mitigação Atual:** O código usa estes ponteiros como observadores não-proprietários, o que é aceitável se os tempos de vida forem gerenciados cuidadosamente.

**Recomendação:** Documentar requisitos de tempo de vida ou considerar usar `std::reference_wrapper<const Note>`.

### 2.3 Problemas de Qualidade Python

#### Uso Incorreto de Comparação com None

**Arquivos afetados:**

- `maialib/maiapy/sethares_dissonance.py:132` → `if amplCallback == None:`
- Outros locais similares

**Correção:**

```python
# Errado
if amplCallback == None:

# Correto
if amplCallback is None:
```

#### Tratamento de Erros Inconsistente

**Problema:** Algumas funções retornam `None` em erro, outras lançam `RuntimeError`.

**Exemplo em `plots.py`:**

```python
# Linha 44: retorna None
if score is None:
    return None

# Linha 36: lança exceção
if score is None:
    raise RuntimeError("Score object is None")
```

**Recomendação:** Padronizar para sempre lançar exceções específicas.

#### Comentários em Português

**Localização e tradução necessária:**

```python
# plots.py:290
# Atual: "# Fixando limite inferior do eixo X em 1"
# Corrigir para: "# Setting X-axis lower limit to 1"

# plots.py:375
# Atual: "# Calcula a dissonância para cada acorde"
# Corrigir para: "# Calculate dissonance for each chord"

# sethares_dissonance.py:375
# Atual: "# Calcula a dissonância de Sethares"
# Corrigir para: "# Calculate Sethares dissonance"
```

### 2.4 Plano de Correção

#### Fase 1: Correções Críticas (Semana 1)

**Prioridade CRÍTICA:**

1. ✅ Corrigir Score copy constructor
2. ✅ Corrigir Score assignment operator
3. ✅ Adicionar testes para verificar cópia correta de Score
4. ✅ Remover debug statement de score.cpp:1671

**Tempo estimado:** 4-6 horas

#### Fase 2: Correções Python (Semana 1-2)

**Prioridade ALTA:**

1. ✅ Substituir `== None` por `is None` em todo código Python
2. ✅ Padronizar tratamento de erros (sempre lançar exceções)
3. ✅ Traduzir todos comentários para inglês
4. ✅ Remover código morto (other.py:107)

**Tempo estimado:** 3-4 horas

#### Fase 3: Executar e Corrigir make validate (Semana 2)

**Prioridade ALTA:**

1. ✅ Executar `make validate` e documentar todos os erros
2. ✅ Corrigir warnings de cpplint (se houver)
3. ✅ Corrigir warnings de cppcheck (se houver)
4. ✅ Adicionar linting Python ao validate

**Tempo estimado:** 4-6 horas

### 2.5 Estimativa Total

**Total de esforço para correções:** 11-16 horas

---

## 3. Cobertura de Testes C++ (maiacore)

### 3.1 Estado Atual: ✅ EXCELENTE

#### Estatísticas Gerais

| Métrica                | Valor             |
| ---------------------- | ----------------- |
| **Arquivos de teste**  | 17 arquivos       |
| **Linhas de teste**    | 9.761 linhas      |
| **Linhas de código**   | ~22.004 linhas    |
| **Razão teste/código** | ~44%              |
| **Framework**          | GoogleTest 1.14.0 |

#### Arquivos de Teste e Cobertura

| Arquivo de Teste          | Linhas | Avaliação                    |
| ------------------------- | ------ | ---------------------------- |
| interval-test.cpp         | 2.150  | ⭐ Excelente - mais completo |
| measure-test.cpp          | 1.133  | ⭐ Excelente                 |
| note-test.cpp             | 839    | ✅ Bom                       |
| duration-test.cpp         | 612    | ✅ Bom                       |
| part-test.cpp             | 514    | ✅ Bom                       |
| score-test.cpp            | 575    | ✅ Bom                       |
| fraction-test.cpp         | 559    | ✅ Bom                       |
| score-collection-test.cpp | 550    | ✅ Bom                       |
| key-test.cpp              | 519    | ✅ Bom                       |
| clef-test.cpp             | 460    | ✅ Bom                       |
| barline-test.cpp          | 441    | ✅ Bom                       |
| time-signature-test.cpp   | 420    | ✅ Bom                       |
| helpers-test.cpp          | 345    | ✅ Bom                       |
| utils-test.cpp            | 266    | ✅ Bom                       |
| chord-test.cpp            | 188    | 🟡 Moderado                  |
| config-test.cpp           | 183    | ✅ Bom                       |

### 3.2 Melhorias Recomendadas

#### Expandir Testes da Classe Chord

**Justificativa:** A classe Chord é uma das mais complexas (2.716 linhas de implementação, 1.548 linhas no header), mas tem apenas 188 linhas de testes.

**Áreas para expansão:**

1. Testes de complexidade harmônica
2. Testes de empilhamento de acordes (chord stacking)
3. Testes de análise de dissonância
4. Testes de intervalos entre notas do acorde
5. Edge cases com acordes grandes (10+ notas)

**Estimativa:** +400-500 linhas de testes

#### Adicionar Testes de Edge Cases

**Categorias de edge cases:**

1. **Scores vazios:** Arquivos XML válidos mas sem conteúdo musical
2. **Scores malformados:** XML inválido ou estrutura incorreta
3. **Valores extremos:** Durações muito longas/curtas, alturas extremas
4. **Unicode e caracteres especiais:** Títulos, nomes de compositores
5. **Performance:** Scores muito grandes (1000+ medidas)

### 3.3 Cobertura de Código

**Recomendação:** Gerar relatório de cobertura com lcov/gcov

**Adicionar ao Makefile:**

```makefile
coverage-report:
	@echo "Generating coverage report..."
	@python scripts/make-run-cpp-tests.py --coverage
	@lcov --capture --directory build --output-file coverage.info
	@lcov --remove coverage.info '/usr/*' '*/external/*' '*/tests-cpp/*' --output-file coverage.info
	@genhtml coverage.info --output-directory coverage-report
	@echo "Coverage report generated in coverage-report/index.html"
```

### 3.4 Plano de Ação

| Tarefa                       | Prioridade | Tempo      |
| ---------------------------- | ---------- | ---------- |
| Expandir chord-test.cpp      | MÉDIA      | 8-10 horas |
| Adicionar edge case tests    | MÉDIA      | 6-8 horas  |
| Configurar lcov/gcov reports | BAIXA      | 3-4 horas  |
| Meta: 50%+ cobertura         | MÉDIA      | -          |

**Total estimado:** 17-22 horas

---

## 4. Cobertura de Testes Python (maialib)

### 4.1 Estado Atual: ⚠️ CRÍTICO

#### Estatísticas Atuais

| Métrica                | Valor      |
| ---------------------- | ---------- |
| **Arquivos de teste**  | 4 arquivos |
| **Linhas de teste**    | 737 linhas |
| **Cobertura estimada** | ~5%        |
| **Framework**          | unittest   |

#### Arquivos de Teste Existentes

| Arquivo         | Linhas | Cobertura      |
| --------------- | ------ | -------------- |
| test_helpers.py | 398    | Funções helper |
| test_note.py    | 162    | Básico Note    |
| test_chord.py   | 152    | Básico Chord   |
| test_score.py   | 25     | ⚠️ MÍNIMO      |

### 4.2 Classes Python SEM Testes

#### Bindings C++ (Crítico)

- ❌ **Score** (apenas 25 linhas - insuficiente!)
- ❌ **Part** (SEM testes)
- ❌ **Measure** (SEM testes)
- ❌ **Interval** (SEM testes)
- ❌ **ScoreCollection** (SEM testes)
- ❌ **Duration** (SEM testes)
- ❌ **Key** (SEM testes)
- ❌ **TimeSignature** (SEM testes)
- ❌ **Clef** (SEM testes)
- ❌ **Barline** (SEM testes)

#### Módulos Python Puros (Alto)

- ❌ **maiapy/plots.py** (SEM testes)
- ❌ **maiapy/sethares_dissonance.py** (SEM testes)
- ❌ **maiapy/other.py** (SEM testes)

### 4.3 Plano de Implementação de Testes Python

#### Fase 1: Testes de Classes Principais (Semanas 1-2)

**Prioridade CRÍTICA**

##### 1. test_score_comprehensive.py (NOVO)

**Escopo:** ~500-600 linhas de testes

```python
import unittest
import maialib as ml
from pathlib import Path

class ScoreTestCase(unittest.TestCase):
    """Comprehensive Score class tests"""

    # Testes de carregamento
    def test_load_valid_xml(self)
    def test_load_compressed_mxl(self)
    def test_load_invalid_xml(self)
    def test_load_nonexistent_file(self)

    # Testes de propriedades
    def test_get_title(self)
    def test_set_title(self)
    def test_get_composer(self)
    def test_set_composer(self)
    def test_get_num_parts(self)
    def test_get_num_measures(self)
    def test_get_num_notes(self)

    # Testes de navegação
    def test_get_part_by_index(self)
    def test_get_part_by_name(self)
    def test_iterate_measures(self)
    def test_iterate_notes(self)

    # Testes de análise
    def test_get_all_notes(self)
    def test_get_all_chords(self)
    def test_get_note_events(self)
    def test_get_duration_seconds(self)

    # Testes de manipulação
    def test_transpose_score(self)
    def test_add_part(self)
    def test_remove_part(self)
    def test_add_measure(self)

    # Testes de exportação
    def test_export_xml(self)
    def test_export_preserves_structure(self)

    # Testes de cópia (CRÍTICO após fix do bug)
    def test_copy_constructor(self)
    def test_assignment_operator(self)
    def test_deep_copy_independence(self)

    # Testes de edge cases
    def test_empty_score(self)
    def test_score_with_unicode_title(self)
    def test_very_large_score(self)
```

**Tempo estimado:** 12-15 horas

##### 2. test_part.py (NOVO)

**Escopo:** ~200-250 linhas

```python
class PartTestCase(unittest.TestCase):
    """Part class tests"""

    def test_get_part_name(self)
    def test_set_part_name(self)
    def test_get_num_measures(self)
    def test_get_measure_by_index(self)
    def test_iterate_measures(self)
    def test_get_all_notes_in_part(self)
    def test_transpose_part(self)
    def test_part_duration(self)
```

**Tempo estimado:** 4-5 horas

##### 3. test_measure.py (NOVO)

**Escopo:** ~200-250 linhas

```python
class MeasureTestCase(unittest.TestCase):
    """Measure class tests"""

    def test_get_measure_number(self)
    def test_get_time_signature(self)
    def test_get_key_signature(self)
    def test_get_clef(self)
    def test_get_notes_in_measure(self)
    def test_add_note(self)
    def test_remove_note(self)
    def test_measure_duration(self)
    def test_is_anacrusis(self)
```

**Tempo estimado:** 4-5 horas

##### 4. test_interval.py (NOVO)

**Escopo:** ~300-350 linhas

```python
class IntervalTestCase(unittest.TestCase):
    """Interval class tests"""

    def test_create_interval_from_notes(self)
    def test_interval_semitones(self)
    def test_interval_name(self)
    def test_interval_quality(self)
    def test_interval_inversion(self)
    def test_interval_addition(self)
    def test_interval_comparison(self)
    def test_consonance_dissonance(self)
    def test_simple_compound_intervals(self)
```

**Tempo estimado:** 6-7 horas

##### 5. test_score_collection.py (NOVO)

**Escopo:** ~250-300 linhas

```python
class ScoreCollectionTestCase(unittest.TestCase):
    """ScoreCollection class tests"""

    def test_create_empty_collection(self)
    def test_add_score_to_collection(self)
    def test_remove_score_from_collection(self)
    def test_iterate_scores(self)
    def test_batch_analysis(self)
    def test_statistical_analysis(self)
    def test_export_collection_data(self)
```

**Tempo estimado:** 5-6 horas

#### Fase 2: Testes de Classes Auxiliares (Semanas 3-4)

**Prioridade ALTA**

##### 6. test_duration.py (NOVO)

**Escopo:** ~150-200 linhas

```python
class DurationTestCase(unittest.TestCase):
    """Duration class tests"""

    def test_create_duration(self)
    def test_duration_to_float(self)
    def test_duration_comparison(self)
    def test_duration_arithmetic(self)
    def test_duration_to_string(self)
```

**Tempo estimado:** 3-4 horas

##### 7. Expandir test_note.py

**Adicionar:** ~100-150 linhas

- Testes de microtones
- Testes de ornamentos
- Testes de articulação
- Edge cases (alturas extremas)

**Tempo estimado:** 3-4 horas

##### 8. Expandir test_chord.py

**Adicionar:** ~150-200 linhas

- Testes de análise harmônica
- Testes de inversões
- Testes de chord stacking
- Testes de complexidade
- Testes de dissonância

**Tempo estimado:** 4-5 horas

##### 9. test_key_time_clef.py (NOVO)

**Escopo:** ~200-250 linhas

```python
class KeyTestCase(unittest.TestCase):
    """Key signature tests"""
    # Testes de tonalidade

class TimeSignatureTestCase(unittest.TestCase):
    """Time signature tests"""
    # Testes de fórmula de compasso

class ClefTestCase(unittest.TestCase):
    """Clef tests"""
    # Testes de clave

class BarlineTestCase(unittest.TestCase):
    """Barline tests"""
    # Testes de barra de compasso
```

**Tempo estimado:** 4-5 horas

#### Fase 3: Testes de Módulos Python Puros (Semanas 5-6)

**Prioridade MÉDIA-ALTA**

##### 10. test_plots.py (NOVO)

**Escopo:** ~300-400 linhas

```python
class PlotsTestCase(unittest.TestCase):
    """Tests for plotting functions"""

    def test_plot_piano_roll(self)
    def test_plot_pitch_envelope(self)
    def test_plot_chord_analysis(self)
    def test_plot_parts_activity(self)
    def test_plot_combines_visualizations(self)
    def test_plot_with_invalid_score(self)
    def test_plot_empty_score(self)

    # Testes que verificam estrutura do retorno
    # (não renderização visual)
```

**Tempo estimado:** 6-8 horas

##### 11. test_sethares_dissonance.py (NOVO)

**Escopo:** ~250-300 linhas

```python
class SetharesDissonanceTestCase(unittest.TestCase):
    """Tests for Sethares dissonance calculations"""

    def test_dissonance_two_tones(self)
    def test_dissonance_curve(self)
    def test_dissonance_chord(self)
    def test_dissonance_dyad_heatmap(self)
    def test_custom_timbre(self)
    def test_zero_amplitude(self)
    def test_known_values(self)  # Validar contra valores publicados
```

**Tempo estimado:** 5-6 horas

##### 12. test_other.py (NOVO)

**Escopo:** ~100-150 linhas

```python
class OtherUtilsTestCase(unittest.TestCase):
    """Tests for utility functions"""

    def test_get_sample_score_path(self)
    def test_get_xml_samples_dir(self)
    def test_set_score_editor_app(self)
    def test_open_score(self)
```

**Tempo estimado:** 2-3 horas

#### Fase 4: Configurar Cobertura de Código (Semana 7)

**Prioridade ALTA**

##### Configurar pytest-cov

**Adicionar ao Makefile:**

```makefile
py-tests-coverage:
	@echo "Running Python tests with coverage..."
	@cd test && python -m pytest --cov=maialib --cov-report=html --cov-report=term
	@echo "Coverage report generated in test/htmlcov/index.html"
```

**Instalar dependências:**

```bash
pip install pytest pytest-cov
```

**Meta de cobertura:** 80%+ para bindings, 70%+ para módulos puros

**Tempo estimado:** 3-4 horas

### 4.4 Resumo do Plano de Testes Python

| Fase       | Arquivos        | Linhas            | Tempo      | Prioridade |
| ---------- | --------------- | ----------------- | ---------- | ---------- |
| **Fase 1** | 5 arquivos      | ~1.800            | 31-38h     | CRÍTICA    |
| **Fase 2** | 4 arquivos      | ~600              | 14-18h     | ALTA       |
| **Fase 3** | 3 arquivos      | ~700              | 13-17h     | MÉDIA-ALTA |
| **Fase 4** | Config          | -                 | 3-4h       | ALTA       |
| **TOTAL**  | **12 arquivos** | **~3.100 linhas** | **61-77h** | -          |

**Meta final:** De 737 linhas → ~3.800+ linhas de testes Python (aumento de 5x)

---

## 5. Testes de Integração

### 5.1 Estado Atual

- ❌ **Não existem testes de integração dedicados**
- Os testes atuais são unitários (testam classes isoladamente)
- Falta validação de workflows end-to-end

### 5.2 Cenários de Teste de Integração

#### Categoria 1: Workflows de Análise Completos

**test_integration_analysis.py** (~400-500 linhas)

```python
class IntegrationAnalysisTestCase(unittest.TestCase):
    """End-to-end analysis workflows"""

    def test_load_analyze_export_workflow(self):
        """Load XML → Analyze → Modify → Export → Reload"""
        # 1. Carregar score
        # 2. Extrair acordes
        # 3. Calcular estatísticas
        # 4. Modificar score
        # 5. Exportar
        # 6. Recarregar e validar

    def test_multi_score_batch_analysis(self):
        """ScoreCollection with multiple scores"""
        # 1. Criar coleção
        # 2. Adicionar múltiplos scores
        # 3. Análise em lote
        # 4. Comparação estatística

    def test_pattern_finding_workflow(self):
        """Find patterns across multiple parts"""
        # 1. Carregar score complexo
        # 2. Definir padrão melódico
        # 3. Buscar em todas as partes
        # 4. Extrair contexto

    def test_harmonic_analysis_workflow(self):
        """Complete harmonic analysis"""
        # 1. Extrair acordes
        # 2. Calcular complexidade
        # 3. Analisar dissonância
        # 4. Gerar visualizações
```

#### Categoria 2: Integração com Visualizações

**test_integration_visualization.py** (~300-400 linhas)

```python
class IntegrationVisualizationTestCase(unittest.TestCase):
    """Integration between analysis and plotting"""

    def test_score_to_piano_roll_pipeline(self):
        """Load score → Extract notes → Generate piano roll"""

    def test_chord_extraction_to_plot(self):
        """Extract chords → Analyze → Plot chord analysis"""

    def test_dissonance_analysis_pipeline(self):
        """Extract chords → Calculate dissonance → Plot curves"""

    def test_multi_plot_dashboard(self):
        """Generate complete analysis dashboard"""
```

#### Categoria 3: Performance e Grandes Datasets

**test_integration_performance.py** (~200-300 linhas)

```python
class IntegrationPerformanceTestCase(unittest.TestCase):
    """Performance tests with large scores"""

    def test_large_score_loading(self):
        """Load score with 500+ measures"""
        # Validar tempo < threshold

    def test_large_score_analysis(self):
        """Analyze score with 10,000+ notes"""

    def test_batch_processing_100_scores(self):
        """Process 100 scores in collection"""

    def test_memory_usage(self):
        """Monitor memory during large operations"""
```

#### Categoria 4: Compatibilidade de Formatos

**test_integration_formats.py** (~200-300 linhas)

```python
class IntegrationFormatsTestCase(unittest.TestCase):
    """Test different MusicXML formats and encodings"""

    def test_compressed_mxl_workflow(self):
        """Work with .mxl files"""

    def test_uncompressed_xml_workflow(self):
        """Work with .xml files"""

    def test_musicxml_workflow(self):
        """Work with .musicxml files"""

    def test_unicode_handling(self):
        """Scores with unicode titles/composers"""

    def test_various_music_notation_software(self):
        """Files from Finale, Sibelius, MuseScore, Dorico"""
```

### 5.3 Plano de Implementação

| Arquivo                           | Linhas          | Tempo      | Prioridade |
| --------------------------------- | --------------- | ---------- | ---------- |
| test_integration_analysis.py      | 400-500         | 10-12h     | ALTA       |
| test_integration_visualization.py | 300-400         | 8-10h      | MÉDIA      |
| test_integration_performance.py   | 200-300         | 6-8h       | MÉDIA      |
| test_integration_formats.py       | 200-300         | 6-8h       | BAIXA      |
| **TOTAL**                         | **1.100-1.500** | **30-38h** | -          |

### 5.4 Infraestrutura Necessária

**Criar diretório:**

```
test/integration/
├── __init__.py
├── test_analysis_workflows.py
├── test_visualization_integration.py
├── test_performance.py
├── test_formats.py
└── fixtures/
    ├── large_scores/
    └── various_formats/
```

**Adicionar ao Makefile:**

```makefile
integration-tests:
	@echo "Running integration tests..."
	@cd test && python -m unittest discover -s integration -p "test_*.py"
	@echo "Integration tests complete."

all-tests: cpp-tests py-tests integration-tests
	@echo "All tests complete."
```

---

## 6. Documentação Completa

### 6.1 Estado Atual da Documentação

#### Documentação Existente: ✅ EXCELENTE

| Documento       | Status        | Qualidade              |
| --------------- | ------------- | ---------------------- |
| README.md       | ✅ Completo   | Excelente (667 linhas) |
| CONTRIBUTING.md | ✅ Completo   | Excelente              |
| BUILDING.md     | ✅ Presente   | Bom                    |
| CHANGELOG.md    | ✅ Atualizado | Bom (v1.10.0)          |
| CLAUDE.md       | ✅ Presente   | Bom                    |

#### Documentação Inline

| Tipo                    | Status      | Qualidade           |
| ----------------------- | ----------- | ------------------- |
| **Doxygen (C++)**       | ✅ Completo | Excelente           |
| **Docstrings (Python)** | ✅ Bom      | Bom (pode melhorar) |
| **Type hints (Python)** | ✅ Presente | Bom                 |

### 6.2 Lacunas na Documentação

#### 1. Falta Hospedagem de Documentação API

**Problema:** Doxygen configurado mas docs não estão online

**Solução:** Configurar GitHub Pages ou ReadTheDocs

**Benefícios:**

- Documentação sempre acessível
- Versionamento automático
- Busca de API
- Aparência profissional

#### 2. Falta Diagrama de Arquitetura

**Problema:** Não há visualização da estrutura do projeto

**Solução:** Criar diagrama de arquitetura em `docs/architecture.md`

**Conteúdo sugerido:**

```markdown
# Arquitetura Maialib

## Visão Geral em Camadas
```

┌─────────────────────────────────────────┐
│ Aplicações do Usuário (Python) │
├─────────────────────────────────────────┤
│ maialib (Python Package) │
│ ├── maiacore (C++ bindings via pybind) │
│ └── maiapy (Pure Python utilities) │
├─────────────────────────────────────────┤
│ maiacore (C++17 Core Library) │
│ ├── Score, Note, Chord, Interval │
│ ├── Part, Measure, Duration │
│ └── Helper functions │
├─────────────────────────────────────────┤
│ External Dependencies │
│ ├── pugixml (XML parsing) │
│ ├── SQLiteCpp (database) │
│ └── nlohmann/json (JSON) │
└─────────────────────────────────────────┘

```

## Fluxo de Dados

## Principais Classes e Relacionamentos
```

#### 3. Falta Documentação de Performance

**Criar:** `docs/performance.md`

**Conteúdo:**

- Benchmarks de carregamento de scores
- Tempo de análise por tamanho de score
- Uso de memória
- Estratégias de otimização
- Cache de cálculos

#### 4. Falta Guia de Migração

**Criar:** `docs/migration_guides/`

**Conteúdo:**

- Guias de migração entre versões
- Breaking changes
- Deprecated features

#### 5. FAQ Expandido

**Expandir README.md ou criar `docs/FAQ.md`**

**Questões comuns:**

- Como converter MIDI para MusicXML?
- Como lidar com scores muito grandes?
- Como integrar com outras bibliotecas musicais?
- Como contribuir com novos recursos?
- Troubleshooting de erros comuns

### 6.3 Melhorias em Docstrings Python

**Exemplo de melhoria:**

```python
# Atual (bom)
def plotPianoRoll(score, **kwargs):
    """Plot piano roll visualization"""
    pass

# Melhorado (excelente)
def plotPianoRoll(
    score: Score,
    partNames: Optional[List[str]] = None,
    showTitle: bool = True,
    **kwargs
) -> Optional[go.Figure]:
    """
    Generate an interactive piano roll visualization of a musical score.

    A piano roll displays notes as horizontal bars, with pitch on the Y-axis
    and time on the X-axis. This visualization is useful for analyzing
    rhythmic patterns, melodic contours, and voice interactions.

    Args:
        score: A maialib Score object containing the musical data to visualize
        partNames: Optional list of part names to include. If None, all parts
            are included in the visualization
        showTitle: If True, displays the score title above the plot
        **kwargs: Additional arguments passed to Plotly's Figure constructor

    Returns:
        A Plotly Figure object containing the piano roll visualization,
        or None if the score is invalid or contains no notes

    Raises:
        RuntimeError: If the score object is None or invalid
        ValueError: If partNames contains invalid part names

    Example:
        >>> import maialib as ml
        >>> score = ml.Score('bach_invention.xml')
        >>> fig = ml.plotPianoRoll(score, partNames=['Piano'])
        >>> fig.show()  # Display in browser
        >>> fig.write_image('piano_roll.png')  # Save as image

    See Also:
        - plotPitchEnvelope: For continuous pitch representation
        - plotPartsActivity: For temporal activity visualization

    Notes:
        - Colors are automatically assigned to different parts
        - Interactive features include zoom, pan, and hover tooltips
        - For large scores (>1000 notes), rendering may take several seconds
    """
    pass
```

---

## 7. Arquitetura e Modularidade

### 7.1 Avaliação Arquitetural: ✅ SÓLIDA

#### Pontos Fortes

1. **Separação clara de responsabilidades:**

   - C++17 core (maiacore) → Lógica de negócio
   - Python bindings → Interface Python
   - Pure Python (maiapy) → Utilidades de alto nível

2. **Bom uso de padrões modernos:**

   - RAII para gerenciamento de recursos
   - STL containers (vector, map, string)
   - Const-correctness na maioria dos métodos
   - Smart pointers onde apropriado

3. **Hierarquia de classes lógica:**

   ```
   Score
   ├── Part[]
   │   └── Measure[]
   │       └── Note[]
   └── Chord[] (derivados)
   ```

4. **Encapsulamento adequado:**
   - Membros privados com getters/setters
   - Interfaces públicas bem definidas

### 7.2 Oportunidades de Refatoração

#### Problema 1: Classes Muito Grandes

##### 1.1 Classe Chord (1.548 linhas de header, 2.716 linhas de implementação)

**Problema:** Responsabilidades misturadas

**Proposta de refatoração:**

```cpp
// Dividir em 3 classes:

// 1. ChordCore - Representação básica
class ChordCore {
    // Notas constituintes
    // Getters/setters básicos
    // Ordenação
};

// 2. ChordAnalysis - Análise harmônica
class ChordAnalysis {
    // Análise de intervalos
    // Complexidade harmônica
    // Classificação de acordes
    // Análise de dissonância
};

// 3. ChordStack - Empilhamento vertical
class ChordStack {
    // Chord stacking
    // Vertical sonorities
    // Harmonic density
};

// Facade para compatibilidade
class Chord : public ChordCore {
    ChordAnalysis _analysis;
    ChordStack _stack;
    // Delegar métodos apropriadamente
};
```

**Benefícios:**

- Testabilidade melhorada
- Código mais legível
- Manutenção facilitada
- Permite reutilização de componentes

**Tempo estimado:** 20-25 horas

##### 1.2 Classe Score (731 linhas de header, 3.291 linhas de implementação)

**Problema:** Mistura I/O, análise e manipulação

**Proposta de refatoração:**

```cpp
// Dividir em classes especializadas:

// 1. ScoreIO - Leitura/escrita de arquivos
class ScoreIO {
    static Score load(const std::string& path);
    static void save(const Score& score, const std::string& path);
    // Parsing XML
    // Serialização
};

// 2. ScoreAnalysis - Análise musical
class ScoreAnalysis {
    // Extração de acordes
    // Análise estatística
    // Pattern finding
    // Note events
};

// 3. ScoreManipulation - Modificação de scores
class ScoreManipulation {
    // Transposição
    // Adição/remoção de partes
    // Modificação de medidas
};

// 4. Score - Classe principal (menor)
class Score {
    ScoreData _data;  // Dados básicos
    // Delegates para classes especializadas
};
```

**Benefícios:**

- Single Responsibility Principle
- Mais fácil testar cada aspecto
- Clareza de intenção

**Tempo estimado:** 30-35 horas

##### 1.3 Classe Helper (529 linhas de header, 2.796 linhas de implementação)

**Problema:** Funções não relacionadas agrupadas

**Proposta de refatoração:**

```cpp
// Dividir em helpers especializados:

// 1. PitchHelper
namespace PitchHelper {
    int midiToStep(int midi);
    std::string midiToNoteName(int midi);
    int noteNameToMidi(const std::string& name);
    // Funções relacionadas a altura
}

// 2. RhythmHelper
namespace RhythmHelper {
    double durationToSeconds(Duration dur, double tempo);
    Duration quarterNoteDuration();
    // Funções relacionadas a ritmo
}

// 3. XMLHelper
namespace XMLHelper {
    pugi::xml_node findNode(/*...*/);
    std::string extractAttribute(/*...*/);
    // Funções de navegação XML
}

// 4. ConversionHelper
namespace ConversionHelper {
    std::string intToString(int val);
    int stringToInt(const std::string& str);
    // Conversões genéricas
}
```

**Benefícios:**

- Organização por domínio
- Facilita descoberta de funções
- Evita conflitos de nomes

**Tempo estimado:** 15-20 horas

#### Problema 2: Tratamento de Erros Inconsistente

**Situação atual:**

- Algumas funções retornam valores default em erro
- Outras lançam exceções
- Falta consistência

**Proposta:**

```cpp
// Opção 1: std::optional para casos recuperáveis
std::optional<Part> getPart(int index) const {
    if (index < 0 || index >= _numParts) {
        return std::nullopt;
    }
    return _part[index];
}

// Opção 2: Exceções para erros graves
class MaialibException : public std::exception {
    // Base exception
};

class InvalidScoreException : public MaialibException {
    // Score inválido
};

class FileNotFoundException : public MaialibException {
    // Arquivo não encontrado
};

// Uso:
Score Score::load(const std::string& path) {
    if (!fileExists(path)) {
        throw FileNotFoundException(path);
    }
    // ...
}
```

**Benefícios:**

- Tratamento de erros mais claro
- Código cliente pode decidir como reagir
- Melhor para APIs públicas

**Tempo estimado:** 10-12 horas

#### Problema 3: Cache Manual

**Situação atual:**

```cpp
mutable std::vector<NoteEvent> _cachedNoteEvents;
mutable bool _isNoteEventsCached;
```

**Proposta:** Criar classe Cache genérica

```cpp
template<typename T>
class LazyCache {
private:
    mutable std::optional<T> _cache;
    std::function<T()> _generator;

public:
    LazyCache(std::function<T()> gen) : _generator(gen) {}

    const T& get() const {
        if (!_cache) {
            _cache = _generator();
        }
        return *_cache;
    }

    void invalidate() { _cache.reset(); }
};

// Uso na classe Score:
LazyCache<std::vector<NoteEvent>> _noteEventsCache;
```

**Benefícios:**

- Elimina código boilerplate
- Reutilizável
- Thread-safe (pode adicionar mutex)

**Tempo estimado:** 6-8 horas

### 7.3 Melhorias de Design Patterns

#### Padrão Builder para Score

**Problema:** Construção complexa de scores

**Solução:**

```cpp
class ScoreBuilder {
public:
    ScoreBuilder& setTitle(const std::string& title);
    ScoreBuilder& setComposer(const std::string& composer);
    ScoreBuilder& addPart(const Part& part);
    ScoreBuilder& setTimeSignature(int num, int denom);
    ScoreBuilder& setKeySignature(const Key& key);
    Score build();
};

// Uso:
Score score = ScoreBuilder()
    .setTitle("My Composition")
    .setComposer("Composer Name")
    .addPart(Part("Piano"))
    .setTimeSignature(4, 4)
    .build();
```

**Tempo estimado:** 8-10 horas

#### Visitor Pattern para Análise

**Problema:** Muitos métodos de análise na classe Score

**Solução:**

```cpp
class ScoreVisitor {
public:
    virtual void visit(const Note& note) = 0;
    virtual void visit(const Chord& chord) = 0;
    virtual void visit(const Measure& measure) = 0;
    virtual ~ScoreVisitor() = default;
};

class StatisticalAnalysisVisitor : public ScoreVisitor {
    // Implementa análise estatística
};

class HarmonicAnalysisVisitor : public ScoreVisitor {
    // Implementa análise harmônica
};

// Uso:
StatisticalAnalysisVisitor visitor;
score.accept(visitor);
auto stats = visitor.getResults();
```

**Tempo estimado:** 12-15 horas

### 7.4 Plano de Refatoração

| Tarefa           | Complexidade | Tempo        | Prioridade |
| ---------------- | ------------ | ------------ | ---------- |
| Refatorar Chord  | Alta         | 20-25h       | MÉDIA      |
| Refatorar Score  | Alta         | 30-35h       | MÉDIA      |
| Refatorar Helper | Média        | 15-20h       | BAIXA      |
| Padronizar erros | Média        | 10-12h       | ALTA       |
| Cache genérico   | Baixa        | 6-8h         | BAIXA      |
| Builder pattern  | Baixa        | 8-10h        | BAIXA      |
| Visitor pattern  | Média        | 12-15h       | BAIXA      |
| **TOTAL**        | -            | **101-125h** | -          |

**Nota:** Refatorações devem ser feitas incrementalmente, uma de cada vez, com testes completos após cada mudança.

---

## 8. Bugs Potenciais no Código C++

### 8.1 Bugs Críticos (Já Documentados na Seção 2)

Consulte **Seção 2.2** para detalhes completos sobre:

- 🔴 Bug 1: Copy constructor incompleto
- 🔴 Bug 2: Assignment operator incompleto
- 🟠 Bug 3: Debug statement em produção
- 🟡 Code Smell 1: Raw pointers

### 8.2 Análise Adicional de Code Smells

#### 1. Uso de Mutable em Classes

**Localização:** Múltiplas classes usam `mutable` para cache

**Exemplo:**

```cpp
mutable std::vector<NoteEvent> _cachedNoteEvents;
mutable bool _isNoteEventsCached;
```

**Avaliação:** ✅ Uso apropriado

- Padrão comum para lazy evaluation
- Não afeta semântica externa da classe
- Funciona bem em contexto single-threaded

**Recomendação:** Adicionar thread-safety se necessário no futuro

#### 2. Comparação de Floats

**Buscar por:** Comparações diretas de `double` com `==`

**Problema potencial:**

```cpp
if (duration == 0.0) {  // Pode ser problemático
```

**Solução:**

```cpp
const double EPSILON = 1e-9;
if (std::abs(duration - 0.0) < EPSILON) {
```

**Ação:** Verificar todo código para comparações de float

**Tempo estimado:** 4-6 horas

#### 3. Alocação de Memória em Loops

**Verificar:** Loops que criam objetos temporários

**Exemplo problemático:**

```cpp
for (int i = 0; i < 10000; ++i) {
    std::string temp = convertToString(i);  // Alocação repetida
    // ...
}
```

**Solução:**

```cpp
std::string temp;
temp.reserve(50);  // Pre-alocar
for (int i = 0; i < 10000; ++i) {
    temp = convertToString(i);
    // ...
}
```

#### 4. Exceptions em Construtores

**Buscar por:** Constructores que podem lançar exceções

**Problema:** Se construtor falha parcialmente, recursos podem vazar

**Solução:** Usar RAII e smart pointers para garantir cleanup

#### 5. Missing Virtual Destructors

**Buscar por:** Classes base sem destrutor virtual

**Exemplo problemático:**

```cpp
class Base {
public:
    ~Base() { }  // Deveria ser virtual
};

class Derived : public Base {
    int* data;
public:
    ~Derived() { delete[] data; }
};

Base* ptr = new Derived();
delete ptr;  // Leak! Destrutor de Derived não é chamado
```

**Ação:** Verificar todas as hierarquias de classes

#### 6. Sinais e Underflows de Inteiros

**Buscar por:** Operações aritméticas sem verificação de overflow

**Exemplo problemático:**

```cpp
int total = note1.duration() + note2.duration();  // Overflow?
```

**Solução:** Usar tipos maiores ou verificar limites

### 8.3 Análise de Segurança de Memória

#### ✅ Pontos Positivos

1. **Sem new/delete manual:** Código usa RAII e containers STL
2. **Smart pointers:** Usado onde apropriado
3. **RAII consistente:** Recursos gerenciados automaticamente
4. **Sem buffer overflows óbvios:** Uso de std::string e std::vector

#### 🟡 Áreas para Revisão

1. **Parsing XML:** Verificar se pugixml protege contra XML bombs
2. **Conversões de string:** Verificar validação de entrada
3. **Índices de array:** Alguns acessos não verificam bounds

### 8.4 Ferramentas de Análise Estática Recomendadas

#### Adicionar ao Makefile:

```makefile
# Static Analysis Tools
clang-tidy:
	@echo "Running clang-tidy..."
	@find maiacore/src -name "*.cpp" | xargs clang-tidy -checks='*'

sanitize-address:
	@echo "Building with AddressSanitizer..."
	@cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" ...
	@make cpp-tests
	@./build/cpp-tests

sanitize-undefined:
	@echo "Building with UndefinedBehaviorSanitizer..."
	@cmake -DCMAKE_CXX_FLAGS="-fsanitize=undefined" ...
	@make cpp-tests
	@./build/cpp-tests

valgrind:
	@echo "Running valgrind..."
	@valgrind --leak-check=full ./build/cpp-tests
```

### 8.5 Plano de Correção de Bugs

| Categoria               | Ações                   | Tempo      | Prioridade |
| ----------------------- | ----------------------- | ---------- | ---------- |
| Bugs críticos (Seção 2) | Corrigir 3 bugs         | 4-6h       | CRÍTICA    |
| Comparações float       | Adicionar epsilon       | 4-6h       | ALTA       |
| Virtual destructors     | Revisar hierarquias     | 3-4h       | ALTA       |
| Adicionar ferramentas   | clang-tidy, sanitizers  | 4-6h       | MÉDIA      |
| Análise completa        | Rodar todas ferramentas | 6-8h       | MÉDIA      |
| Correções adicionais    | Baseado em resultados   | 10-15h     | VARIÁVEL   |
| **TOTAL**               | -                       | **31-45h** | -          |

---

## 9. Tutoriais e Exemplos Práticos

### 9.1 Estado Atual: ✅ EXCELENTE BASE

#### Inventário Completo

**Total:** 25 notebooks organizados em 5 categorias

**Estrutura:**

```
python-tutorial/
├── README.md (Comprehensive learning path)
├── 00_getting_started/ (2 notebooks) ✅
├── 01_beginner/ (4 notebooks) 🟡 50% completo
├── 02_intermediate/ (2 notebooks) 🟡 20% completo
├── 03_advanced/ (2 notebooks) 🟡 33% completo
├── 04_visualization/ (5 notebooks) ✅ 83% completo
└── 05_analysis/ (0 notebooks) ❌ 0% completo
```

### 9.2 Notebooks Faltantes (21 tutoriais)

#### 00_Getting_Started (1 faltante)

- ❌ 02_library_overview.ipynb

#### 01_Beginner (4 faltantes)

- ❌ 01_notes_basics.ipynb
- ❌ 02_chords_basics.ipynb
- ❌ 04_loading_scores.ipynb
- ❌ 05_creating_simple_scores.ipynb
- ❌ 07_duration_and_rhythm.ipynb

#### 02_Intermediate (8 faltantes)

- ❌ 01_score_navigation.ipynb
- ❌ 04_transposed_instruments.ipynb
- ❌ 05_time_key_signatures.ipynb
- ❌ 06_multi_part_scores.ipynb
- ❌ 07_score_editing.ipynb
- ❌ 08_advanced_chord_properties.ipynb
- ❌ 09_melodic_analysis.ipynb
- ❌ 10_harmonic_rhythm.ipynb

#### 03_Advanced (4 faltantes)

- ❌ 03_score_collections.ipynb
- ❌ 04_statistical_analysis.ipynb
- ❌ 05_algorithmic_composition.ipynb
- ❌ 06_custom_analysis_functions.ipynb

#### 04_Visualization (1 faltante)

- ❌ 05_dissonance_analysis.ipynb

#### 05_Analysis (3 faltantes - categoria inteira)

- ❌ 01_harmonic_complexity.ipynb
- ❌ 02_dissonance_curves.ipynb
- ❌ 03_corpus_statistics.ipynb

### 9.3 Plano de Criação de Tutoriais

#### Prioridade 1: Completar Beginner (5 notebooks)

##### 01_notes_basics.ipynb

**Conteúdo:**

1. Criando notas programaticamente
2. Propriedades de notas (pitch, duration, octave)
3. Modificando notas
4. Comparando notas
5. Exemplos práticos: escalas cromáticas

**Exemplos musicais:**

- Escala cromática
- Escala maior
- Arpejo simples

**Tempo:** 4-5 horas

##### 02_chords_basics.ipynb

**Conteúdo:**

1. Criando acordes a partir de notas
2. Propriedades de acordes
3. Inversões
4. Análise básica de intervalos
5. Exemplos: tríades maiores, menores, aumentadas, diminutas

**Exemplos musicais:**

- Progressão I-IV-V-I
- Cadências harmônicas
- Círculo das quintas

**Tempo:** 4-5 horas

##### 04_loading_scores.ipynb

**Conteúdo:**

1. Carregar arquivos XML, MXL
2. Inspecionar propriedades do score
3. Navegar partes e medidas
4. Extrair informações básicas
5. Tratamento de erros

**Exemplos musicais:**

- Bach Invenções
- Mozart Sonatas (trechos)
- Beethoven (trechos simples)

**Tempo:** 3-4 horas

##### 05_creating_simple_scores.ipynb

**Conteúdo:**

1. Criar score do zero
2. Adicionar partes
3. Adicionar medidas
4. Adicionar notas
5. Exportar para MusicXML
6. Abrir em software de notação

**Exemplos musicais:**

- "Ode to Joy" (Beethoven)
- "Twinkle Twinkle Little Star"
- Melodia original simples

**Tempo:** 5-6 horas

##### 07_duration_and_rhythm.ipynb

**Conteúdo:**

1. Durações (whole, half, quarter, etc.)
2. Tuplets (triolinas, quintinas)
3. Pontos de aumento
4. Pausas
5. Fórmulas de compasso

**Exemplos musicais:**

- Ritmos sincopados
- Padrões rítmicos complexos
- Polirritmos simples

**Tempo:** 4-5 horas

**TOTAL Beginner:** 20-25 horas

#### Prioridade 2: Completar Intermediate (8 notebooks)

##### 01_score_navigation.ipynb

- Iterar notas, medidas, partes
- XPath básico
- Filtrar elementos
  **Tempo:** 4-5h

##### 04_transposed_instruments.ipynb

- Transposição
- Instrumentos transpositores
- Concert pitch vs written pitch
  **Tempo:** 3-4h

##### 05_time_key_signatures.ipynb

- Mudanças de fórmula de compasso
- Mudanças de armadura
- Anacruses
  **Tempo:** 3-4h

##### 06_multi_part_scores.ipynb

- Trabalhar com partituras orquestrais
- Extrair partes específicas
- Analisar relações entre partes
  **Tempo:** 4-5h

##### 07_score_editing.ipynb

- Modificar scores existentes
- Adicionar/remover elementos
- Transposição
- Exportar versões editadas
  **Tempo:** 5-6h

##### 08_advanced_chord_properties.ipynb

- Complexidade harmônica
- Stacked chords
- Análise vertical
  **Tempo:** 4-5h

##### 09_melodic_analysis.ipynb

- Contornos melódicos
- Análise de intervalos melódicos
- Pontos de culminância
  **Tempo:** 5-6h

##### 10_harmonic_rhythm.ipynb

- Taxa de mudança harmônica
- Análise de ritmo harmônico
- Visualizações
  **Tempo:** 4-5h

**TOTAL Intermediate:** 32-40 horas

#### Prioridade 3: Completar Advanced (4 notebooks)

##### 03_score_collections.ipynb

- Criar coleções de scores
- Análise em lote
- Comparações estatísticas
- Corpus analysis basics
  **Tempo:** 5-6h

##### 04_statistical_analysis.ipynb

- Distribuições de alturas
- Estatísticas de intervalos
- Análise de diversidade
- Testes estatísticos
  **Tempo:** 6-7h

##### 05_algorithmic_composition.ipynb

- Cadeias de Markov
- Algoritmos generativos
- Composição baseada em regras
- Exportar composições
  **Tempo:** 8-10h

##### 06_custom_analysis_functions.ipynb

- Criar funções de análise personalizadas
- Integrar com pipeline existente
- Visualizar resultados
  **Tempo:** 5-6h

**TOTAL Advanced:** 24-29 horas

#### Prioridade 4: Completar Visualization e Analysis (4 notebooks)

##### 04_visualization/05_dissonance_analysis.ipynb

- Curvas de dissonância de Sethares
- Heatmaps de dissonância
- Análise de timbre
  **Tempo:** 5-6h

##### 05_analysis/01_harmonic_complexity.ipynb

- Métricas de complexidade
- Análise de densidade harmônica
- Comparações entre obras
  **Tempo:** 5-6h

##### 05_analysis/02_dissonance_curves.ipynb

- Dissonância ao longo do tempo
- Tensão e resolução
- Análise de obras completas
  **Tempo:** 5-6h

##### 05_analysis/03_corpus_statistics.ipynb

- Análise de corpus musical
- Estatísticas comparativas
- Descoberta de padrões
- Visualizações de corpus
  **Tempo:** 6-8h

**TOTAL Viz+Analysis:** 21-26 horas

### 9.4 Novos Tutoriais Propostos (Além do Roadmap)

#### Tutoriais Práticos de Análise Musical

##### advanced/07_modal_analysis.ipynb

**Novo tutorial sobre análise modal**

**Conteúdo:**

- Identificação de modos (Dórico, Frígio, etc.)
- Análise de música modal vs tonal
- Detecção de centros tonais
- Exemplos: música medieval, jazz modal, música folclórica

**Tempo:** 6-8h

##### advanced/08_form_analysis.ipynb

**Novo tutorial sobre análise formal**

**Conteúdo:**

- Detecção de seções (ABA, Sonata, Rondó)
- Análise de repetições
- Identificação de temas
- Visualização de estrutura formal

**Tempo:** 7-9h

##### analysis/04_performance_analysis.ipynb

**Novo tutorial sobre análise de performance**

**Conteúdo:**

- Comparar múltiplas versões da mesma obra
- Análise de tempo e dinâmica
- Variações interpretativas
- Visualizações comparativas

**Tempo:** 6-8h

##### analysis/05_style_classification.ipynb

**Novo tutorial sobre classificação de estilo**

**Conteúdo:**

- Características estilísticas por período
- Análise comparativa de compositores
- Machine learning básico para classificação
- Feature extraction

**Tempo:** 8-10h

### 9.5 Melhorias nos Tutoriais Existentes

**Ações:**

1. Adicionar células de "Exercícios Práticos" ao final
2. Incluir mais exemplos musicais reconhecíveis
3. Adicionar links para documentação API
4. Melhorar visualizações com anotações
5. Adicionar seção "Troubleshooting" comum

**Tempo total:** 10-15 horas

### 9.6 Limpeza de Notebooks Legacy

**Problema:** 10 notebooks legacy no root de python-tutorial/

**Ação:**

- Mover para `python-tutorial/legacy/`
- Adicionar README.md explicando status
- Marcar como deprecated

**Tempo:** 2-3 horas

### 9.7 Resumo do Plano de Tutoriais

| Fase                      | Notebooks    | Tempo        | Prioridade |
| ------------------------- | ------------ | ------------ | ---------- |
| Completar Beginner        | 5            | 20-25h       | ALTA       |
| Completar Intermediate    | 8            | 32-40h       | MÉDIA      |
| Completar Advanced        | 4            | 24-29h       | MÉDIA      |
| Completar Viz+Analysis    | 4            | 21-26h       | MÉDIA      |
| Novos Tutoriais Avançados | 4            | 27-35h       | BAIXA      |
| Melhorias em Existentes   | -            | 10-15h       | BAIXA      |
| Limpeza Legacy            | -            | 2-3h         | BAIXA      |
| **TOTAL**                 | **25 novos** | **136-173h** | -          |

**Meta:** Crescer de 25 para 50+ notebooks bem documentados

---

## 10. Novas Features para Análise Musical

### 10.1 Análise Tonal Avançada

#### Feature 1: Detecção de Tonalidade com Krumhansl-Schmuckler

**Implementação:**

```cpp
// maiacore/include/maiacore/key_detection.h
class KeyDetection {
public:
    struct KeyProfile {
        Key key;
        double correlation;
        double confidence;
    };

    static std::vector<KeyProfile> detectKey(const Score& score);
    static std::vector<KeyProfile> detectKeyBySegment(
        const Score& score,
        int measureStart,
        int measureEnd
    );

private:
    static std::map<int, double> _majorProfile;
    static std::map<int, double> _minorProfile;
    static double calculateCorrelation(/*...*/);
};
```

**Funcionalidades:**

- Algoritmo Krumhansl-Schmuckler para detecção de tonalidade
- Análise de modulações
- Segmentação tonal automática
- Confidence scores para ambiguidades

**Tempo estimado:** 15-20 horas

#### Feature 2: Análise de Funções Harmônicas

**Implementação:**

```cpp
// maiacore/include/maiacore/harmonic_function.h
enum class HarmonicFunction {
    Tonic,
    Subdominant,
    Dominant,
    Mediant,
    Submediant,
    Leading,
    Diminished,
    Unknown
};

class HarmonicAnalysis {
public:
    struct ChordFunction {
        Chord chord;
        HarmonicFunction function;
        int romanNumeral;
        std::string label;  // e.g., "V7", "ii°"
    };

    static std::vector<ChordFunction> analyzeFunctions(
        const Score& score,
        const Key& key
    );

    static std::vector<std::string> identifyCadences(
        const std::vector<ChordFunction>& functions
    );
};
```

**Funcionalidades:**

- Análise de função harmônica (T, S, D)
- Numeração romana automática
- Detecção de cadências (autêntica, plagal, deceptiva)
- Análise de progressões harmônicas

**Tempo estimado:** 20-25 horas

### 10.2 Análise Modal

#### Feature 3: Identificação de Modos

**Implementação:**

```cpp
// maiacore/include/maiacore/mode_analysis.h
enum class Mode {
    Ionian,      // Major
    Dorian,
    Phrygian,
    Lydian,
    Mixolydian,
    Aeolian,     // Natural Minor
    Locrian,
    Unknown
};

class ModeAnalysis {
public:
    struct ModeProfile {
        Mode mode;
        int tonic;  // MIDI pitch
        double confidence;
    };

    static ModeProfile detectMode(const Score& score);
    static std::vector<int> getModeScale(Mode mode, int tonic);
    static bool isModalMusic(const Score& score);

    // Análise de música não-tonal
    static std::map<int, double> getPitchClassDistribution(
        const Score& score
    );
};
```

**Funcionalidades:**

- Detecção de modos gregos
- Análise de centros modais
- Suporte para música folclórica e jazz modal
- Pitch class set analysis

**Tempo estimado:** 18-22 horas

### 10.3 Estatística Musical para Pesquisa

#### Feature 4: Análise Estatística Avançada

**Implementação:**

```cpp
// maiacore/include/maiacore/statistics.h
class MusicStatistics {
public:
    struct IntervalStatistics {
        std::map<int, int> melodicIntervalCounts;
        std::map<int, int> harmonicIntervalCounts;
        double averageMelodicInterval;
        double intervalVariance;
        std::vector<int> mostCommonIntervals;
    };

    struct RhythmStatistics {
        std::map<double, int> durationHistogram;
        double averageNoteDuration;
        double rhythmicVariety;  // Entropy
        std::vector<std::vector<double>> rhythmicPatterns;
    };

    struct PitchStatistics {
        std::map<int, int> pitchClassHistogram;
        double pitchRange;
        int highestPitch;
        int lowestPitch;
        double averagePitch;
        std::vector<int> mostCommonPitches;
    };

    static IntervalStatistics analyzeIntervals(const Score& score);
    static RhythmStatistics analyzeRhythm(const Score& score);
    static PitchStatistics analyzePitch(const Score& score);

    // Diversidade e complexidade
    static double calculateMelodicEntropy(const Score& score);
    static double calculateRhythmicEntropy(const Score& score);
    static double calculateHarmonicComplexity(const Score& score);
};
```

**Funcionalidades:**

- Histogramas de alturas, intervalos, durações
- Métricas de diversidade (entropia)
- Análise de complexidade
- Estatísticas descritivas completas

**Tempo estimado:** 25-30 horas

#### Feature 5: Análise de Corpus

**Implementação:**

```cpp
// maiacore/include/maiacore/corpus_analysis.h
class CorpusAnalysis {
public:
    struct CorpusStatistics {
        int totalScores;
        int totalNotes;
        std::map<std::string, double> averagesByComposer;
        std::map<int, double> temporalTrends;
    };

    CorpusAnalysis(const ScoreCollection& corpus);

    // Comparações
    std::map<std::string, double> compareComposers(
        const std::string& feature
    );

    std::vector<Score> findSimilarScores(
        const Score& reference,
        int topN
    );

    // Clustering
    std::map<int, std::vector<Score>> clusterByStyle(int numClusters);

    // Export
    void exportToDataFrame(const std::string& csvPath);
};
```

**Funcionalidades:**

- Análise de grandes corpus musicais
- Comparações entre compositores
- Descoberta de padrões em corpus
- Clustering por similaridade
- Export para análise em Python/R

**Tempo estimado:** 30-35 horas

### 10.4 Comparação de Obras Musicais

#### Feature 6: Métricas de Similaridade

**Implementação:**

```cpp
// maiacore/include/maiacore/similarity.h
class SimilarityMetrics {
public:
    struct SimilarityScore {
        double overall;
        double melodic;
        double harmonic;
        double rhythmic;
        double structural;
    };

    static SimilarityScore compare(
        const Score& score1,
        const Score& score2
    );

    // Métricas específicas
    static double melodicSimilarity(
        const std::vector<Note>& melody1,
        const std::vector<Note>& melody2
    );

    static double harmonicSimilarity(
        const std::vector<Chord>& chords1,
        const std::vector<Chord>& chords2
    );

    static double rhythmicSimilarity(
        const Score& score1,
        const Score& score2
    );

    // Algoritmos
    static double levenshteinDistance(
        const std::vector<int>& seq1,
        const std::vector<int>& seq2
    );

    static double cosineSimilarity(
        const std::map<int, double>& vec1,
        const std::map<int, double>& vec2
    );
};
```

**Funcionalidades:**

- Múltiplas métricas de similaridade
- Comparação melódica (contorno, intervalos)
- Comparação harmônica (progressões)
- Comparação rítmica (padrões)
- Edit distance algorithms

**Tempo estimado:** 25-30 horas

### 10.5 Análise de Contraponto

#### Feature 7: Análise Contrapontística

**Implementação:**

```cpp
// maiacore/include/maiacore/counterpoint.h
class CounterpointAnalysis {
public:
    enum class VoiceLeadingRule {
        ParallelFifths,
        ParallelOctaves,
        HiddenFifths,
        DirectOctaves,
        VoiceCrossing,
        VoiceOverlap,
        LeapResolution,
        DissonanceTreatment
    };

    struct VoiceLeadingViolation {
        VoiceLeadingRule rule;
        int measure;
        double beat;
        std::vector<int> voices;
        std::string description;
    };

    static std::vector<VoiceLeadingViolation> analyzeVoiceLeading(
        const Score& score,
        const std::set<VoiceLeadingRule>& rulesToCheck
    );

    static double calculateIndependenceScore(
        const Part& part1,
        const Part& part2
    );

    static std::map<std::string, int> countMotionTypes(
        const Score& score
    );  // Parallel, contrary, oblique, similar
};
```

**Funcionalidades:**

- Detecção de quintas e oitavas paralelas
- Análise de condução de vozes
- Verificação de regras de contraponto
- Métricas de independência de vozes
- Análise de tipos de movimento

**Tempo estimado:** 20-25 horas

### 10.6 Análise de Forma Musical

#### Feature 8: Detecção de Estrutura Formal

**Implementação:**

```cpp
// maiacore/include/maiacore/form_analysis.h
class FormAnalysis {
public:
    struct Section {
        std::string label;  // "A", "B", "Development", etc.
        int startMeasure;
        int endMeasure;
        double similarity;  // With other sections
    };

    static std::vector<Section> detectSections(const Score& score);

    static std::string identifyForm(
        const std::vector<Section>& sections
    );  // "ABA", "Sonata", "Rondo", etc.

    static std::vector<int> findRepetitions(
        const Score& score,
        int minLength
    );

    static std::map<int, int> buildSimilarityMatrix(
        const Score& score,
        int windowSize
    );
};
```

**Funcionalidades:**

- Segmentação automática
- Detecção de repetições e variações
- Identificação de forma (Binary, Ternary, Sonata, Rondo)
- Matriz de auto-similaridade
- Análise temática

**Tempo estimado:** 25-30 horas

### 10.7 Importação de Arquivos MIDI

#### Feature 9: MIDI Import para Score

**Implementação:**

```cpp
// maiacore/include/maiacore/midi_import.h
class MidiImport {
public:
    struct MidiImportOptions {
        int divisionsPerQuarterNote = 480;  // Resolução temporal
        bool quantize = false;              // Quantização automática
        double quantizeGrid = 0.25;         // Grade de quantização (em beats)
        bool inferTimeSignature = true;     // Inferir fórmula de compasso
        bool inferKeySignature = true;      // Inferir armadura de clave
        bool splitByTracks = true;          // Uma parte por track MIDI
        std::string defaultClef = "G";      // Clave padrão
    };

    struct MidiMetadata {
        std::string title;
        std::string composer;
        int tempo;  // BPM
        int timeNumerator;
        int timeDenominator;
        std::vector<std::string> trackNames;
    };

    /**
     * Importa arquivo MIDI e converte para Score
     *
     * @param midiFilePath Caminho para o arquivo .mid ou .midi
     * @param options Opções de importação
     * @return Score object contendo a música importada
     * @throws MidiImportException se o arquivo for inválido
     */
    static Score importFromMidi(
        const std::string& midiFilePath,
        const MidiImportOptions& options = MidiImportOptions()
    );

    /**
     * Extrai metadados do arquivo MIDI sem carregar todo o score
     */
    static MidiMetadata extractMetadata(const std::string& midiFilePath);

    /**
     * Converte MIDI track individual para Part
     */
    static Part midiTrackToPart(
        const std::vector<MidiEvent>& events,
        const MidiImportOptions& options
    );

private:
    // Estrutura interna para eventos MIDI
    struct MidiEvent {
        int track;
        int tick;           // Tempo em ticks MIDI
        int type;           // Note On, Note Off, etc.
        int channel;
        int note;           // MIDI note number (0-127)
        int velocity;
        int duration;       // Duração em ticks
    };

    // Parsing de arquivo MIDI
    static std::vector<MidiEvent> parseMidiFile(const std::string& path);

    // Conversão de ticks MIDI para Duration
    static Duration ticksToDuration(
        int ticks,
        int divisionsPerQuarterNote,
        int midiDivision
    );

    // Inferência de tonalidade a partir de notas MIDI
    static Key inferKey(const std::vector<MidiEvent>& events);

    // Inferência de fórmula de compasso
    static TimeSignature inferTimeSignature(
        const std::vector<MidiEvent>& events
    );

    // Quantização de durações
    static Duration quantizeDuration(
        const Duration& original,
        double grid
    );

    // Análise de andamento
    static int extractTempo(const std::vector<MidiEvent>& events);
};

// Exceção específica para erros de importação MIDI
class MidiImportException : public std::runtime_error {
public:
    explicit MidiImportException(const std::string& message)
        : std::runtime_error("MIDI Import Error: " + message) {}
};
```

**Implementação Python Binding:**

```cpp
// maiacore/src/maiacore/python_wrapper/py_midi_import.cpp
void init_midi_import(py::module &m) {
    py::class_<MidiImport::MidiImportOptions>(m, "MidiImportOptions")
        .def(py::init<>())
        .def_readwrite("divisions_per_quarter_note",
                      &MidiImport::MidiImportOptions::divisionsPerQuarterNote)
        .def_readwrite("quantize",
                      &MidiImport::MidiImportOptions::quantize)
        .def_readwrite("quantize_grid",
                      &MidiImport::MidiImportOptions::quantizeGrid)
        .def_readwrite("infer_time_signature",
                      &MidiImport::MidiImportOptions::inferTimeSignature)
        .def_readwrite("infer_key_signature",
                      &MidiImport::MidiImportOptions::inferKeySignature)
        .def_readwrite("split_by_tracks",
                      &MidiImport::MidiImportOptions::splitByTracks);

    py::class_<MidiImport>(m, "MidiImport")
        .def_static("import_from_midi",
                   &MidiImport::importFromMidi,
                   py::arg("midi_file_path"),
                   py::arg("options") = MidiImport::MidiImportOptions(),
                   "Import MIDI file and convert to Score")
        .def_static("extract_metadata",
                   &MidiImport::extractMetadata,
                   "Extract metadata from MIDI file");
}
```

**Uso em Python:**

```python
import maialib as ml

# Importação básica
score = ml.MidiImport.import_from_midi("bach_invention.mid")

# Importação com opções customizadas
options = ml.MidiImportOptions()
options.quantize = True
options.quantize_grid = 0.125  # Quantizar para semicolcheias
options.infer_key_signature = True
options.split_by_tracks = True

score = ml.MidiImport.import_from_midi("symphony.mid", options)

# Extrair apenas metadados
metadata = ml.MidiImport.extract_metadata("concerto.mid")
print(f"Title: {metadata.title}")
print(f"Composer: {metadata.composer}")
print(f"Tempo: {metadata.tempo} BPM")

# Exportar para MusicXML após importação
score.toFile("output.xml")
```

**Funcionalidades:**

- ✅ Importação de arquivos MIDI (formato 0, 1 e 2)
- ✅ Conversão automática de eventos MIDI para estrutura Score/Part/Measure/Note
- ✅ Inferência inteligente de tonalidade e fórmula de compasso
- ✅ Quantização opcional para limpeza de timing
- ✅ Preservação de dinâmicas (via velocity)
- ✅ Separação por tracks MIDI (multi-instrumento)
- ✅ Extração de metadados (título, compositor, tempo)
- ✅ Conversão de números MIDI para notação musical (C4, D#5, etc.)
- ✅ Suporte a diferentes claves por instrumento
- ✅ Exportação para MusicXML após importação

**Bibliotecas MIDI a considerar:**

- **midifile** (C++): https://github.com/craigsapp/midifile - Leve, header-only, MIT license
- **RtMidi** (C++): Para I/O MIDI em tempo real (se necessário no futuro)

**Tempo estimado:** 30-35 horas

**Casos de uso:**

1. **Pesquisadores:** Importar corpus MIDI existente para análise com maialib
2. **Compositores:** Importar performances MIDI de DAWs para análise
3. **Educadores:** Converter exercícios MIDI em partituras editáveis
4. **Desenvolvedores:** Pipeline MIDI → MusicXML → Análise → Visualização

### 10.8 Análise de Tensão e Relaxamento

#### Feature 10: Modelagem de Tensão Musical

**Implementação:**

```cpp
// maiacore/include/maiacore/tension.h
class TensionAnalysis {
public:
    struct TensionCurve {
        std::vector<double> timePoints;
        std::vector<double> tensionValues;
    };

    // Múltiplos modelos de tensão
    static TensionCurve calculateHarmonicTension(
        const Score& score
    );

    static TensionCurve calculateMelodicTension(
        const Score& score
    );

    static TensionCurve calculateRhythmicTension(
        const Score& score
    );

    static TensionCurve calculateOverallTension(
        const Score& score,
        const std::map<std::string, double>& weights
    );

    // Análise de pontos críticos
    static std::vector<int> findClimax(const TensionCurve& curve);
    static std::vector<int> findRelaxation(const TensionCurve& curve);
};
```

**Funcionalidades:**

- Modelagem de tensão harmônica
- Modelagem de tensão melódica
- Modelagem de tensão rítmica
- Curvas combinadas
- Identificação de clímax e resoluções
- Visualizações

**Tempo estimado:** 20-25 horas

### 10.9 Resumo de Novas Features

| Feature            | Categoria     | Tempo        | Prioridade | Impacto |
| ------------------ | ------------- | ------------ | ---------- | ------- |
| Key Detection      | Teoria        | 15-20h       | ALTA       | ALTO    |
| Harmonic Functions | Teoria        | 20-25h       | ALTA       | ALTO    |
| Mode Analysis      | Teoria        | 18-22h       | MÉDIA      | MÉDIO   |
| Music Statistics   | Pesquisa      | 25-30h       | ALTA       | ALTO    |
| Corpus Analysis    | Pesquisa      | 30-35h       | MÉDIA      | ALTO    |
| Similarity Metrics | Comparação    | 25-30h       | MÉDIA      | MÉDIO   |
| Counterpoint       | Análise       | 20-25h       | BAIXA      | MÉDIO   |
| Form Analysis      | Análise       | 25-30h       | BAIXA      | MÉDIO   |
| MIDI Import        | I/O & Interop | 30-35h       | ALTA       | ALTO    |
| Tension Modeling   | Análise       | 20-25h       | BAIXA      | MÉDIO   |
| **TOTAL**          | -             | **228-277h** | -          | -       |

### 10.10 Roadmap de Implementação de Features

#### Fase 1: Fundamentos Teóricos e I/O (2-3 meses)

- MIDI Import (prioridade para expandir fontes de entrada)
- Key Detection
- Harmonic Functions
- Music Statistics
  **Total:** 90-110 horas

#### Fase 2: Pesquisa e Corpus (2-3 meses)

- Corpus Analysis
- Similarity Metrics
  **Total:** 55-65 horas

#### Fase 3: Análises Avançadas (2-3 meses)

- Mode Analysis
- Form Analysis
- Tension Modeling
  **Total:** 63-77 horas

#### Fase 4: Especialização (1-2 meses)

- Counterpoint Analysis
  **Total:** 20-25 horas

---

## 11. Roadmap de Implementação Consolidado

### Fase 1: Fundamentos e Correções (Semanas 1-2)

**Objetivo:** Corrigir problemas críticos e estabelecer infraestrutura de qualidade

| Tarefa                            | Tempo      | Responsável |
| --------------------------------- | ---------- | ----------- |
| Corrigir bugs críticos C++        | 4-6h       | Dev C++     |
| Corrigir code smells Python       | 3-4h       | Dev Python  |
| Adicionar `make format` targets   | 1h         | DevOps      |
| Criar pyproject.toml              | 30min      | DevOps      |
| Configurar pre-commit hooks       | 30min      | DevOps      |
| Formatar todo código              | 1h         | Automático  |
| Traduzir comentários para inglês  | 2h         | Dev         |
| Criar test_score_comprehensive.py | 12-15h     | Dev Python  |
| **TOTAL FASE 1**                  | **24-30h** | -           |

**Entregáveis:**

- ✅ Bugs críticos corrigidos
- ✅ Código totalmente formatado
- ✅ Pipeline de formatação automatizado
- ✅ Testes abrangentes para Score

### Fase 2: Expansão de Testes Python (Semanas 3-6)

**Objetivo:** Alcançar 80%+ cobertura de testes Python

| Tarefa                            | Tempo      |
| --------------------------------- | ---------- |
| Criar test_part.py                | 4-5h       |
| Criar test_measure.py             | 4-5h       |
| Criar test_interval.py            | 6-7h       |
| Criar test_score_collection.py    | 5-6h       |
| Criar test_duration.py            | 3-4h       |
| Expandir test_note.py             | 3-4h       |
| Expandir test_chord.py            | 4-5h       |
| Criar test_key_time_clef.py       | 4-5h       |
| Criar test_plots.py               | 6-8h       |
| Criar test_sethares_dissonance.py | 5-6h       |
| Criar test_other.py               | 2-3h       |
| Configurar pytest-cov             | 3-4h       |
| **TOTAL FASE 2**                  | **49-62h** |

**Entregáveis:**

- ✅ 12 novos arquivos de teste Python
- ✅ ~3.800 linhas de testes
- ✅ 80%+ cobertura de código Python
- ✅ Relatórios de cobertura automatizados

### Fase 3: Testes de Integração (Semanas 7-8)

**Objetivo:** Validar workflows end-to-end

| Tarefa                                  | Tempo      |
| --------------------------------------- | ---------- |
| Criar test_integration_analysis.py      | 10-12h     |
| Criar test_integration_visualization.py | 8-10h      |
| Criar test_integration_performance.py   | 6-8h       |
| Criar test_integration_formats.py       | 6-8h       |
| Adicionar fixtures de teste             | 3-4h       |
| **TOTAL FASE 3**                        | **33-42h** |

**Entregáveis:**

- ✅ Suite de testes de integração
- ✅ Testes de performance
- ✅ Validação de formatos

### Fase 4: Documentação e Hospedagem (Semanas 9-10)

**Objetivo:** Documentação profissional e acessível

| Tarefa                              | Tempo      |
| ----------------------------------- | ---------- |
| Configurar GitHub Pages/ReadTheDocs | 4-6h       |
| Criar docs/architecture.md          | 6-8h       |
| Criar docs/performance.md           | 8-10h      |
| Melhorar docstrings Python          | 10-12h     |
| Criar docs/FAQ.md                   | 4-6h       |
| Criar guias de migração             | 2-3h       |
| **TOTAL FASE 4**                    | **34-45h** |

**Entregáveis:**

- ✅ Documentação API online
- ✅ Diagramas de arquitetura
- ✅ Documentação de performance
- ✅ FAQ completo

### Fase 5: Tutoriais - Nível Iniciante (Semanas 11-12)

**Objetivo:** Completar tutoriais beginner

| Tarefa                          | Tempo      |
| ------------------------------- | ---------- |
| 01_notes_basics.ipynb           | 4-5h       |
| 02_chords_basics.ipynb          | 4-5h       |
| 04_loading_scores.ipynb         | 3-4h       |
| 05_creating_simple_scores.ipynb | 5-6h       |
| 07_duration_and_rhythm.ipynb    | 4-5h       |
| **TOTAL FASE 5**                | **20-25h** |

**Entregáveis:**

- ✅ 5 novos tutoriais beginner
- ✅ Categoria 01_beginner completa

### Fase 6: Tutoriais - Nível Intermediário (Semanas 13-16)

**Objetivo:** Completar tutoriais intermediate

| Tarefa                         | Tempo      |
| ------------------------------ | ---------- |
| 8 novos notebooks intermediate | 32-40h     |
| **TOTAL FASE 6**               | **32-40h** |

**Entregáveis:**

- ✅ 8 novos tutoriais intermediate
- ✅ Categoria 02_intermediate completa

### Fase 7: Tutoriais - Níveis Avançado e Análise (Semanas 17-20)

**Objetivo:** Completar todos tutoriais restantes

| Tarefa                                | Tempo       |
| ------------------------------------- | ----------- |
| 4 notebooks advanced                  | 24-29h      |
| 4 notebooks visualization/analysis    | 21-26h      |
| 4 notebooks novos (modal, form, etc.) | 27-35h      |
| Melhorar tutoriais existentes         | 10-15h      |
| Limpeza legacy                        | 2-3h        |
| **TOTAL FASE 7**                      | **84-108h** |

**Entregáveis:**

- ✅ Todas categorias de tutoriais completas
- ✅ 4 novos tutoriais avançados
- ✅ 50+ notebooks totais

### Fase 8: Refatoração de Arquitetura (Meses 6-8)

**Objetivo:** Melhorar modularidade e manutenibilidade

| Tarefa                         | Tempo        |
| ------------------------------ | ------------ |
| Refatorar Chord class          | 20-25h       |
| Refatorar Score class          | 30-35h       |
| Refatorar Helper classes       | 15-20h       |
| Padronizar tratamento de erros | 10-12h       |
| Implementar cache genérico     | 6-8h         |
| Adicionar Builder pattern      | 8-10h        |
| Adicionar Visitor pattern      | 12-15h       |
| **TOTAL FASE 8**               | **101-125h** |

**Entregáveis:**

- ✅ Código refatorado e modular
- ✅ Design patterns aplicados
- ✅ Tratamento de erros consistente

### Fase 9: Novas Features - Teoria Musical (Meses 9-11)

**Objetivo:** Adicionar análises teóricas avançadas

| Tarefa             | Tempo      |
| ------------------ | ---------- |
| Key Detection      | 15-20h     |
| Harmonic Functions | 20-25h     |
| Mode Analysis      | 18-22h     |
| Music Statistics   | 25-30h     |
| **TOTAL FASE 9**   | **78-97h** |

**Entregáveis:**

- ✅ Detecção automática de tonalidade
- ✅ Análise de funções harmônicas
- ✅ Análise modal
- ✅ Estatísticas musicais avançadas

### Fase 10: Novas Features - Pesquisa e Comparação (Meses 12-14)

**Objetivo:** Ferramentas para pesquisa musicológica

| Tarefa                | Tempo        |
| --------------------- | ------------ |
| Corpus Analysis       | 30-35h       |
| Similarity Metrics    | 25-30h       |
| Counterpoint Analysis | 20-25h       |
| Form Analysis         | 25-30h       |
| Tension Modeling      | 20-25h       |
| **TOTAL FASE 10**     | **120-145h** |

**Entregáveis:**

- ✅ Análise de corpus musicais
- ✅ Métricas de similaridade
- ✅ Análise contrapontística
- ✅ Detecção de forma musical
- ✅ Modelagem de tensão

---

## 12. Estimativa Total de Esforço

### Resumo por Categoria

| Categoria                  | Horas Estimadas | Percentual |
| -------------------------- | --------------- | ---------- |
| **Formatação e Qualidade** | 5-6h            | 1%         |
| **Correção de Bugs**       | 31-45h          | 4%         |
| **Testes C++**             | 17-22h          | 2%         |
| **Testes Python**          | 61-77h          | 8%         |
| **Testes de Integração**   | 30-38h          | 4%         |
| **Documentação**           | 32-42h          | 4%         |
| **Tutoriais**              | 136-173h        | 18%        |
| **Refatoração**            | 101-125h        | 13%        |
| **Novas Features**         | 228-277h        | 28%        |
| **TOTAL**                  | **641-805h**    | **100%**   |

### Cronograma Estimado

**Em tempo integral (40h/semana):** 16-20 semanas (~4-5 meses)

**Em meio período (20h/semana):** 32-40 semanas (~8-10 meses)

**Em 10% do tempo (4h/semana):** 160-201 semanas (~3-4 anos)

### Distribuição de Prioridades

| Prioridade  | Tarefas | Horas    | Percentual |
| ----------- | ------- | -------- | ---------- |
| **CRÍTICA** | 3       | 80-107h  | 13%        |
| **ALTA**    | 7       | 157-200h | 26%        |
| **MÉDIA**   | 11      | 274-333h | 42%        |
| **BAIXA**   | 8       | 100-130h | 19%        |

---

## 13. Matriz de Prioridades Detalhada

### Tarefas CRÍTICAS (Fazer Imediatamente)

| #   | Tarefa                                      | Tempo  | Justificativa                       |
| --- | ------------------------------------------- | ------ | ----------------------------------- |
| 1   | Corrigir Score copy constructor/assignment  | 4-6h   | Bug grave, pode causar crashes      |
| 2   | Criar test_score_comprehensive.py           | 12-15h | Score é classe mais importante      |
| 3   | Adicionar automação de formatação           | 5-6h   | Fundamento para qualidade de código |
| 4   | Criar testes Python para classes principais | 49-62h | Essencial para confiabilidade       |
| 5   | Configurar cobertura de código Python       | 3-4h   | Métricas de qualidade               |

**Total CRÍTICO:** 73-93 horas

### Tarefas ALTAS (Fazer em Seguida)

| #   | Tarefa                               | Tempo  | Justificativa              |
| --- | ------------------------------------ | ------ | -------------------------- |
| 6   | Correções Python (None, erros, etc.) | 3-4h   | Qualidade de código        |
| 7   | Executar e corrigir make validate    | 4-6h   | Garantia de qualidade      |
| 8   | Testes de integração                 | 30-38h | Validação end-to-end       |
| 9   | Hospedar documentação API            | 4-6h   | Acessibilidade             |
| 10  | Melhorar docstrings Python           | 10-12h | Developer experience       |
| 11  | Completar tutoriais beginner         | 20-25h | Onboarding de usuários     |
| 12  | Padronizar tratamento de erros       | 10-12h | Robustez                   |
| 13  | Implementar Key Detection            | 15-20h | Feature muito solicitada   |
| 14  | Implementar Harmonic Functions       | 20-25h | Teoria musical fundamental |
| 15  | Implementar Music Statistics         | 25-30h | Essencial para pesquisa    |

**Total ALTO:** 141-178 horas

### Tarefas MÉDIAS (Fazer Quando Possível)

**Total MÉDIO:** 274-333 horas

Inclui:

- Expandir testes C++
- Tutoriais intermediate/advanced
- Documentação de arquitetura
- Mode Analysis
- Corpus Analysis
- Similarity Metrics
- Refatorações principais

### Tarefas BAIXAS (Futuro)

**Total BAIXO:** 100-130 horas

Inclui:

- Counterpoint Analysis
- Form Analysis
- Tension Modeling
- Refatorações menores
- Tutoriais muito avançados

---

## 14. Métricas de Sucesso

### Indicadores Quantitativos

| Métrica                 | Atual | Meta   | Melhoria |
| ----------------------- | ----- | ------ | -------- |
| Cobertura Testes Python | ~5%   | 80%+   | +15x     |
| Linhas Testes Python    | 737   | 3.800+ | +5x      |
| Notebooks Tutoriais     | 25    | 50+    | +2x      |
| Bugs Críticos           | 3     | 0      | -100%    |
| Formatação Automatizada | Não   | Sim    | +∞       |
| Docs API Online         | Não   | Sim    | +∞       |
| Classes Testadas Python | 4     | 15+    | +3.75x   |
| Features de Análise     | ~20   | 30+    | +50%     |

### Indicadores Qualitativos

- ✅ Código totalmente formatado e consistente
- ✅ Documentação acessível e profissional
- ✅ Tutoriais cobrindo todos níveis
- ✅ Arquitetura modular e manutenível
- ✅ Tratamento de erros consistente
- ✅ Features avançadas para pesquisa musical

---

## 15. Próximos Passos Imediatos

### Semana 1: Ações Imediatas

**Dia 1-2: Correções Críticas**

1. ✅ Corrigir Score copy constructor
2. ✅ Corrigir Score assignment operator
3. ✅ Remover debug statement
4. ✅ Adicionar testes para cópia de Score

**Dia 3-4: Infraestrutura de Formatação**

1. ✅ Criar pyproject.toml
2. ✅ Adicionar targets make format
3. ✅ Formatar todo código
4. ✅ Traduzir comentários

**Dia 5: Testes Python - Início**

1. ✅ Começar test_score_comprehensive.py
2. ✅ Implementar testes básicos de Score

### Semana 2: Consolidação

**Dia 1-3: Completar Testes Score**

1. ✅ Finalizar test_score_comprehensive.py
2. ✅ Alcançar 80%+ cobertura de Score

**Dia 4-5: Validação**

1. ✅ Executar make validate
2. ✅ Corrigir todos erros
3. ✅ Documentar resultados

---

## 16. Conclusão

Este plano de desenvolvimento fornece um roteiro abrangente para elevar o **Maialib** de um projeto já excelente para um líder absoluto em bibliotecas de análise musical.

### Pontos Fortes Atuais (Manter)

- ✅ C++17 moderno e bem arquitetado
- ✅ Documentação excelente
- ✅ Tutoriais bem organizados
- ✅ Cobertura de testes C++ exemplar

### Áreas de Melhoria (Focar)

- ⚠️ Testes Python insuficientes → Prioridade CRÍTICA
- ⚠️ Falta automação de formatação → Prioridade ALTA
- 🟡 Alguns bugs críticos → Prioridade CRÍTICA
- 🟡 Tutoriais incompletos → Prioridade MÉDIA

### Visão de Longo Prazo

Transformar Maialib em:

1. **Referência em qualidade:** Código impecável, 80%+ cobertura
2. **Ferramenta de pesquisa:** Features avançadas para musicologia
3. **Educacional:** 50+ tutoriais cobrindo todos aspectos
4. **Profissional:** Documentação online, arquitetura exemplar

### Compromisso com Excelência

Este plano representa aproximadamente **641-805 horas** de trabalho focado, mas o resultado será uma biblioteca de análise musical de classe mundial, mantendo a qualidade excepcional já demonstrada no código C++ e expandindo-a para todos os aspectos do projeto.

**Destaques das Melhorias Planejadas:**

- 🎹 **MIDI Import:** Expandir capacidades de entrada com suporte completo a arquivos MIDI
- 📊 **Ruff Integration:** Ferramenta única para formatação e linting Python (substituindo black + isort)
- 🌐 **Documentação Online:** Deploy automático em www.maialib.com via GitHub Pages
- 🧪 **Cobertura de Testes:** De 5% para 80%+ em testes Python
- 📚 **Tutoriais:** Crescer de 25 para 50+ notebooks interativos
- 🎼 **Análise Musical:** 10 novas features avançadas para pesquisa e composição

**Status: Pronto para Execução** 🚀

---

_Documento gerado em: 19 de Novembro de 2025_
_Versão do Plano: 1.1_
_Baseado em: Maialib v1.10.0_
_Atualizado com: MIDI Import, Ruff integration, GitHub Pages (www.maialib.com)_
