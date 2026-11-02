<#
.SYNOPSIS
    Публикация проекта "Night Clock" на GitHub.
.DESCRIPTION
    Скрипт проверяет наличие gh и git, выполняет вход в GitHub (если нужно),
    инициализирует локальный репозиторий, создаёт публичный репозиторий
    и отправляет в него все файлы.
#>

# ── 1. Параметры ──────────────────────────────────────────────
$RepoOwner  = "AdmiralMK"
$RepoName   = "NightClock"                  # имя репозитория (без пробелов)
$ProjectDir = "D:\ArduinoApp\NightClock"    # путь к папке проекта
$CommitMsg  = "Initial commit: Night Clock v1.0"
$Branch     = "main"

# ── 2. Проверка окружения ────────────────────────────────────
Write-Host "→ Проверка окружения..." -ForegroundColor Cyan

if (-not (Get-Command gh -ErrorAction SilentlyContinue)) {
    Write-Error "GitHub CLI (gh) не найден. Установите: https://cli.github.com/"
    exit 1
}
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Error "Git не найден. Установите Git for Windows."
    exit 1
}
if (-not (Test-Path $ProjectDir)) {
    Write-Error "Папка проекта не найдена: $ProjectDir"
    exit 1
}

Set-Location $ProjectDir

# ── 3. Аутентификация GitHub ─────────────────────────────────
Write-Host "→ Проверка авторизации GitHub..." -ForegroundColor Cyan

gh auth status 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "  Требуется вход в GitHub. Откроется браузер..." -ForegroundColor Yellow
    # Неинтерактивный вход через браузер (подтверждение «y»)
    "y" | gh auth login --web --git-protocol https --hostname github.com
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Авторизация GitHub не выполнена."
        exit 1
    }
}
Write-Host "  ✓ Авторизация OK" -ForegroundColor Green

# ── 4. Инициализация Git (если ещё не инициализирован) ──────
if (-not (Test-Path ".git")) {
    Write-Host "→ Инициализация Git-репозитория..." -ForegroundColor Cyan
    git init -b $Branch
    git add .
    git commit -m $CommitMsg
} else {
    Write-Host "→ Git-репозиторий уже существует, коммитим изменения..." -ForegroundColor Cyan
    git add .
    $status = git status --porcelain
    if ($status) {
        git commit -m $CommitMsg
    } else {
        Write-Host "  Новых изменений нет." -ForegroundColor Gray
    }
}

# ── 5. Создание репозитория на GitHub и пуш ─────────────────
Write-Host "→ Создание публичного репозитория $RepoOwner/$RepoName..." -ForegroundColor Cyan

# Проверяем, существует ли уже репозиторий
gh repo view "$RepoOwner/$RepoName" 2>$null
if ($LASTEXITCODE -eq 0) {
    Write-Host "  Репозиторий уже существует. Настраиваем remote и пушим..." -ForegroundColor Yellow
    git remote remove origin 2>$null
    git remote add origin "https://github.com/$RepoOwner/$RepoName.git"
    git push -u origin $Branch
} else {
    # Создаём новый публичный репозиторий из локальной папки и пушим
    gh repo create "$RepoOwner/$RepoName" --public --source=. --remote=origin --push
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Не удалось создать репозиторий."
        exit 1
    }
}

Write-Host "`n✅ Готово! Проект опубликован:" -ForegroundColor Green
Write-Host "   https://github.com/$RepoOwner/$RepoName" -ForegroundColor Cyan