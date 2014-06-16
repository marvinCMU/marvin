<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">

<xsl:template match="/w:document">
\documentclass{article}
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="w:body">
\begin{document}
<xsl:apply-templates/>
\end{document}
</xsl:template>

<xsl:template match="w:p">
<xsl:apply-templates/><xsl:if test="position()!=last()"><xsl:text>

</xsl:text></xsl:if>
</xsl:template>

<xsl:template match="w:r">
<xsl:if test="w:footnoteReference"><xsl:text>\footnote{</xsl:text>
<xsl:call-template name="footnote">
<xsl:with-param name="fid"><xsl:value-of select="//@w:id"/></xsl:with-param>
</xsl:call-template>
<xsl:text>}</xsl:text>
</xsl:if>
<xsl:if test="w:rPr/w:b"><xsl:text>\textbf{</xsl:text></xsl:if>
<xsl:call-template name="pastb"/>
<xsl:if test="w:rPr/w:b"><xsl:text>}</xsl:text></xsl:if>
</xsl:template>

<xsl:template name="pastb">
<xsl:if test="w:rPr/w:i"><xsl:text>\textit{</xsl:text></xsl:if>
<xsl:call-template name="pasti"/>
<xsl:if test="w:rPr/w:i"><xsl:text>}</xsl:text></xsl:if>
</xsl:template>

<xsl:template name="pasti">
<xsl:apply-templates select="w:t"/>
</xsl:template>

<xsl:template name="footnote">
<xsl:param name="fid"/>
<xsl:apply-templates select="document('footnotes.xml')/w:footnotes/w:footnote[@w:id=$fid]"/>
</xsl:template>

<xsl:template match="//w:footnote">
<xsl:apply-templates select="w:p"/>
</xsl:template>

</xsl:stylesheet>
