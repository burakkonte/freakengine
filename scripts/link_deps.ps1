$mainRepo = "C:\Users\su-ko\OneDrive\Masaüstü\freakland-m1"
$worktree = "C:\Users\su-ko\OneDrive\Masaüstü\freakland-m1\.claude\worktrees\keen-panini"

$dirs = @("SDL", "bgfx.cmake", "entt", "glm", "imgui", "json", "stb", "tomlplusplus", "tracy")

foreach ($dir in $dirs) {
    $target = Join-Path $mainRepo "third_party\$dir"
    $link = Join-Path $worktree "third_party\$dir"
    if (-not (Test-Path $link)) {
        Write-Host "Linking $dir..."
        New-Item -ItemType Junction -Path $link -Target $target | Out-Null
    } else {
        Write-Host "$dir already exists"
    }
}

Write-Host "Done."
