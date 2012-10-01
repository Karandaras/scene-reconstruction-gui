#include <fstream>

namespace SceneReconstruction {
/** @class DIKGraph "dikgraph.h"
 *  simple structure to contruct and store the DIK Graph
 *  @author Bastian Klingen
 */
  class DIKGraph {
    public:
      /** simple structure for the edges of the DIK Graph
       */
      struct DIKEdge {
        /** node from which the edge comes */
        std::string from;
        /** node to which the edge points */
        std::string to;
        /** label of the edge */
        std::string label;
        /** equality operator
         *  @param rhs an edge to compare with
         *  @return true if edges are equal
         */
        bool operator==(const DIKEdge& rhs) const
        {
            if(from != rhs.from)
              return false;
            else if(to != rhs.to)
              return false;
            else if(label != rhs.label)
              return false;
            else
              return true;
        }
        /** equality operator
         *  @param rhs string representation of an edge to compare with
         *  @return true if edges are equal
         */
        bool operator==(const std::string& rhs) const
        {
            if(toString() != rhs)
              return false;
            else
              return true;
        }
        /** get the string representation of the edge
         *  @return string representation of this edge
         */
        std::string toString() const
        {
          return "\""+from+"\" --"+(label!=""?"\""+label+"\"":"")+"--> \""+to+"\"";
        }

        /** get the dot representation of the edge's style
         *  @param marked true to mark the edge (bold, red)
         *  @return string representation of this edge
         */
        std::string dot_edge(bool marked) {
          std::string style = "";
          if(!label.empty() || marked) {
            style += " [dir=back";
              if(!label.empty())
                style += ",label=\""+label+"\"";
              if(marked)
                style += ",style=bold,color=red";
            style += "]";
          }

          return style;
        }

      };

      /** simple structure for the nodes of the DIK Graph
       */
      struct DIKNode {
        /** name of the node */
        std::string node;
        /** list of pointers to parent nodes */
        std::list<DIKNode*> parents;
        /** list of pointers to child nodes */
        std::list<DIKNode*> children;

        /** equality operator
         *  only checks for names since the graph does not allow two
         *  nodes with equal names
         *  @param rhs node name
         *  @return true if names are equal
         */
        bool operator==(const std::string& rhs) const
        {
            if(node != rhs)
              return false;
            else
              return true;
        }

        /** get the name of the node
         *  @return name of the node
         */
        std::string toString() const
        {
          return node;
        }
      };

    private:
      /** marks the parents of an node
       *  @param node pointer to a node
       */
      void mark_parents(DIKGraph::DIKNode* node) {
        std::list<DIKGraph::DIKNode*>::iterator iter;
        for(iter = node->parents.begin(); iter != node->parents.end(); iter++) {
          if(!is_marked((*iter)->node)) {
            marked_nodes.push_back((*iter)->node);
            mark_parents(*iter);
          }
        }
      }
      
    public:
      /** save the graph to a file 
       *  @param filename name of the file to save to
       */
      void save(std::string filename) {
        std::ofstream out;
        out.open(filename.c_str(), std::ios::out | std::ios::trunc);
        std::list<DIKNode>::iterator iter;
        for(iter = knowledge_nodes.begin(); iter != knowledge_nodes.end(); iter++)
          out << iter->node << ";";
        out << "\n";
        for(iter = information_nodes.begin(); iter != information_nodes.end(); iter++)
          out << iter->node << ";";
        out << "\n";
        for(iter = data_nodes.begin(); iter != data_nodes.end(); iter++)
          out << iter->node << ";";
        out << "\n";

        std::list<DIKEdge>::iterator eiter;
        for(eiter = edges.begin(); eiter != edges.end(); eiter++) {
          out << eiter->from << ";" << eiter->to << ";" << eiter->label << ";\n";
        }

        out.close();
      }
        
      /** checks if a node exists
       *  @param node name of the node to search for
       *  @return true if node exists, false otherwise
       */
      bool is_node(std::string node) {
        std::list<DIKGraph::DIKNode>::iterator iter;
        for(iter = knowledge_nodes.begin(); iter != knowledge_nodes.end(); iter++)
          if(*iter == node)
            return true;
        for(iter = information_nodes.begin(); iter != information_nodes.end(); iter++)
          if(*iter == node)
            return true;
        for(iter = data_nodes.begin(); iter != data_nodes.end(); iter++)
          if(*iter == node)
            return true;

        return false;
      }

      /** checks if an edge exists
       *  @param edge edge to search for
       *  @return true if edge exists, false otherwise
       */
      bool is_edge(DIKEdge edge) {
        return (find(edges.begin(), edges.end(), edge) != edges.end());
      }

      /** gets the level of a node
       *  @param node node to get the level of
       *  @return level of the node {knowledge,information,data}
       */
      std::string level_of_node(std::string node) {
        if (find(knowledge_nodes.begin(), knowledge_nodes.end(), node) != knowledge_nodes.end())
          return "knowledge";
        else if (find(information_nodes.begin(), information_nodes.end(), node) != information_nodes.end())
          return "information";
        else if (find(data_nodes.begin(), data_nodes.end(), node) != data_nodes.end())
          return "data";
        else
          return "";
      }

      /** marks the children of an node
       *  @param node pointer to a node
       */
      void mark_children(DIKNode* node) {
        std::list<DIKNode*>::iterator iter;
        for(iter = node->children.begin(); iter != node->children.end(); iter++) {
          if(!is_marked((*iter)->node)) {
            marked_nodes.push_back((*iter)->node);
            mark_children(*iter);
          }
        }
      }

      /** returns a pointer to a node specified by its name
       *  @param node name of the node
       *  @return pointer to the node or 0 if node does not exist
       */
      DIKNode* get_node(std::string node) {
        DIKNode* n = 0;
        std::list<DIKNode>::iterator iter;
        iter = find(knowledge_nodes.begin(), knowledge_nodes.end(), node);
        if(iter != knowledge_nodes.end()) {
          n=&(*iter);
          return n;
        }
        iter = find(information_nodes.begin(), information_nodes.end(), node);
        if(iter != information_nodes.end()) {
          n=&(*iter);
          return n;
        }
        iter = find(data_nodes.begin(), data_nodes.end(), node);
        if(iter != data_nodes.end()) {
          n=&(*iter);
          return n;
        }

        return n;
      }

      /** checks if a node is marked
       *  @param node name of the node to search for
       *  @return true if node is marked, false otherwise
       */
      bool is_marked(std::string node) {
        return (find(marked_nodes.begin(), marked_nodes.end(), node) != marked_nodes.end());
      }

      /** creates the dot representation of the graph
       *  @param markupnode the name of the node to mark
       *  @return string containing this graph in dot
       */
      std::string get_dot(std::string markupnode) {
        std::list<DIKGraph::DIKNode>::iterator iter;
        std::list<DIKGraph::DIKEdge>::iterator edgeiter;
        // standard "header" of the graph
        std::string dot = "digraph G {\n"\
                          "  ranksep=1;\n"\
                          "  edge[style=invis];\n"\
                          "  node[shape=box,fontsize=20,fixedsize=true,width=2];\n"\
                          "  \"Knowledge\" -> \"Information\" -> \"Data\";\n"\
                          "  edge[style=solid,dir=back];\n"\
                          "  node[shape=ellipse,fontsize=14,fixedsize=false,width=0.75];\n";

        //find nodes and edges connected to the markupnode
        marked_nodes.clear();
        if(markupnode != "") {
          marked_nodes.push_back(markupnode);
          //mark nodes pointing to marked nodes
          mark_children(get_node(markupnode));

          //mark nodes pointed from marked nodes
          mark_parents(get_node(markupnode));
        }
        
        // create knowledge nodes
        dot            += "  subgraph knowledge {\n"\
                          "    rank = same;\n"\
                          "    \"Knowledge\";\n";
        for(iter = knowledge_nodes.begin(); iter != knowledge_nodes.end(); iter++) {
          dot          += "    \""+(iter->node)+"\""+(is_marked(iter->node)?" [style=bold,color=red]":"")+";\n";
        }
        dot            += "  }\n";

        // create information nodes
        dot            += "  subgraph information {\n"\
                          "    rank = same;\n"\
                          "    \"Information\";\n";
        for(iter = information_nodes.begin(); iter != information_nodes.end(); iter++) {
          dot          += "    \""+(iter->node)+"\""+(is_marked(iter->node)?" [style=bold,color=red]":"")+";\n";
        }
        dot            += "  }\n";

        // create data nodes
        dot            += "  subgraph data {\n"\
                          "    rank = same;\n"\
                          "    \"Data\";\n";
        for(iter = data_nodes.begin(); iter != data_nodes.end(); iter++) {
          dot          += "    \""+(iter->node)+"\""+(is_marked(iter->node)?" [style=bold,color=red]":"")+";\n";
        }
        dot            += "  }\n";

        // create edges
        for(edgeiter = edges.begin(); edgeiter != edges.end(); edgeiter++) {
          dot          += "    \""+(edgeiter->to)+"\" -> \""+(edgeiter->from)+"\""+edgeiter->dot_edge(is_marked(edgeiter->from) && is_marked(edgeiter->to))+";\n";
        }

        // end the graph
        dot            += "}";
        
        return dot;
      }

    public:
      /** list of knowledge nodes */
      std::list<DIKNode>      knowledge_nodes;
      /** list of information nodes */
      std::list<DIKNode>      information_nodes;
      /** list of data nodes */
      std::list<DIKNode>      data_nodes;
      /** list of edges */
      std::list<DIKEdge>      edges;
    
    private:
      /** list of marked nodes */
      std::list<std::string>  marked_nodes;
  };
}

