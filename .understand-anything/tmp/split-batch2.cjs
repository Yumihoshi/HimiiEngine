const fs = require('fs');

const graph = JSON.parse(fs.readFileSync('D:/Himii-Engine/.understand-anything/tmp/ua-batch2-all-graph.json', 'utf8'));
const { nodes, edges, sortedPaths, parts, filesPerPart } = graph;

const outDir = 'D:/Himii-Engine/.understand-anything/intermediate';

function getFilePath(node) {
  if (node.filePath) return node.filePath;
  // For non-file nodes, extract path from the ID
  const id = node.id;
  // Check if it's a file node
  if (id.startsWith('file:')) return id.substring(5);
  // For function:/class: nodes, extract the path part
  const m = id.match(/^(function|class|table|endpoint):(.+?):[^:]+$/);
  if (m) return m[2];
  return null;
}

for (let p = 0; p < parts; p++) {
  const startIdx = p * filesPerPart;
  const endIdx = Math.min(startIdx + filesPerPart, sortedPaths.length);
  const partPaths = new Set(sortedPaths.slice(startIdx, endIdx));

  const partNodes = nodes.filter(n => {
    const fp = getFilePath(n);
    return fp && partPaths.has(fp);
  });

  // Edges where source is in this part's nodes
  const partNodeIds = new Set(partNodes.map(n => n.id));
  const partEdges = edges.filter(e => partNodeIds.has(e.source));

  // Also include edges whose target references a node in this part (but source in another part)
  // We need this so cross-part edges are present in at least one part
  // Actually, the protocol says edges where source is in this part's nodes
  // But to avoid dangling edges, we should include cross-part targets too
  // However, the merge script handles this. Let's follow the spec: edges whose source is in this part.

  const partNum = p + 1;
  const filename = parts === 1
    ? outDir + '/batch-2.json'
    : outDir + '/batch-2-part-' + partNum + '.json';

  fs.writeFileSync(filename, JSON.stringify({ nodes: partNodes, edges: partEdges }, null, 2));
  console.log('Part ' + partNum + ': ' + partNodes.length + ' nodes, ' + partEdges.length + ' edges -> ' + filename);

  // Validate
  for (const e of partEdges) {
    // source must be in this part
    if (!partNodeIds.has(e.source)) {
      console.error('  ERROR: edge source not in part: ' + JSON.stringify(e));
    }
  }
}

console.log('Done writing ' + parts + ' parts.');
