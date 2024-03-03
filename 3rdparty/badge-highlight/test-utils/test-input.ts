export const html = `<!DOCTYPE html>
<section>
    <blockquote><p> [!IMPORTANT] 
    text </p></blockquote>
</section>
<section>
    <blockquote><p> [!NOTE] text </p></blockquote>
</section>
<section>
    <blockquote><p> [!TIP] text </p></blockquote>
</section>
<section>
    <blockquote><p> [!WARNING] text </p></blockquote>
</section>
<section>
    <blockquote><p> [!CAUTION] text </p></blockquote>
</section>
<section>
    <blockquote class="modified"><p> [!NOTE]
     [!TIP] </p></blockquote>
</section>
<section>
    <blockquote class="not-modified"><p> [!TIP] [!NOTE] </p></blockquote>
</section>
<section>
    <p>
        <blockquote class="not-modified-nested"><p> [!TIP] </p></blockquote>
    </p>
</section>
`;
